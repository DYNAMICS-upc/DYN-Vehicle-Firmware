# Consolidated Technical Compliance & Safety Report: Vehicle Firmware Suite

| Parameter | Specification |
| :--- | :--- |
| **Project** | DYNAMICS UPC - Formula Student Electric Vehicle Firmware Suite |
| **Audited Firmware Targets** | **MCU** (Motor Control Unit), **ECU** (Electronic Control Unit), **PDM** (Power Distribution Module) |
| **Production Codebase** | `MCU/MCU_FW/`, `ECU/ECU_FW/`, `PDM/PDM_FW/` (ESP-IDF v5 + FreeRTOS SMP) |
| **Applied Standards** | MISRA-C:2012, ISO 26262 (ASIL-D / ASIL-B Principles), Zero Dynamic Memory, 100 Hz Deterministic FreeRTOS |
| **CI/CD Build & Test Status** | **PASSED** (Compilation with `-Werror`, static analysis, and 100% host unit tests passing) |

---

## 1. Executive Summary & Distributed System Architecture

This report certifies the functional safety, industrial compliance, and quality metrics of the embedded firmware suite powering the **DYNAMICS UPC** Formula Student electric racing vehicle.

```mermaid
graph TD
    subgraph Sensors & Driver Inputs
        APPS[Dual Redundant APPS] --> MCU
        HPS[Hydraulic Brake HPS] --> MCU
        ENC[4x IRAM Wheel Encoders] --> MCU
        STEER[Steering Angle Pot] --> MCU
        NTC[Bosch NTC Thermistors SPI] --> ECU
        STS[4x Suspension Strain Gauges] --> ECU
        SHUNTS[12x Shunts + 2x Hall Sensors] --> PDM
    end

    subgraph Deterministic CAN Bus Network (500 kbps)
        MCU[MCU / VCU Core] <-->|0x020, 0x021, 0x502| BUS[Differential CAN Bus]
        ECU[ECU Cooling Control] <-->|0x401, 0x402, 0x503| BUS
        PDM[PDM LV Power Hub] <-->|0x001..0x006, 0x501| BUS
        BUS <-->|0x0C0 Torque Cmd| INV[Unitek Bamocar Inverter]
        BUS <-->|Telemetry Logging| DASH[Dashboard & Data Logger]
    end

    subgraph Actuators & Power Distribution
        ECU -->|LEDC PWM 14-bit 50 Hz| ESC[Motor & Inverter ESCs]
        PDM -->|12x Solid-State Gate Drivers| MOSFETS[LV Loads: Pumps, Sensors, ECU, Steering Wheel]
        MCU -->|Torque Command & R2D Enable| MOTOR[Electric Traction Motor]
    end
```

---

## 2. Consolidated Vehicle Fault Management Matrix (DTC & Safe States)

Every anomaly across the vehicle is categorized, assigned deterministic hardware safe-state actions, and broadcasted as standard Diagnostic Trouble Codes (DTC):

| Board | DTC Code | Fault Identifier | Cat. | Prio. | Trigger Condition | System & Hardware Reaction | Recovery / Reset | Broadcasted CAN Frame |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **ECU** | **`201`** | `FAULT_CODE_MOTOR_NTC_FAIL` | `HW` | `HIGH` | $\ge 3$ failed reads on Motor NTC. | Ramps Motor cooling fan (+10%/s) to **100% continuous power**. | Automatic when both NTCs read valid temperatures. | ID `0x503` (DTC `201`, Fans at 100%) |
| **ECU** | **`202`** | `FAULT_CODE_INV_NTC_FAIL` | `HW` | `HIGH` | $\ge 3$ failed reads on Inverter NTC. | Ramps Inverter cooling fan to **100% continuous power**. | Automatic once valid temperature returns. | ID `0x503` (DTC `202`) |
| **ECU** | **`203`** | `FAULT_CODE_ADS8688_SPI_ERR` | `HW` | `HIGH` | SPI communication timeout with ADS8688 ADC. | Invalidates telemetry; forces both fans to **100%**. | Requires SPI bus reset or hard reboot. | ID `0x503` (DTC `203`) |
| **ECU** | **`204`** | `FAULT_CODE_TWAI_BUS_OFF` | `COMM` | `HIGH` | CAN controller enters `BUS_OFF` state. | Calls `twai_initiate_recovery()` and restarts CAN driver. | Automatic upon physical bus recovery. | ID `0x503` upon recovery |
| **PDM** | **`10..21`** | `FAULT_CODE_OVERCURRENT_CH(0..11)` | `HW` | `HIGH` | Current on shunted channel $i > 1.30 \times I_{\text{nom}}$ (fast trip or 3 samples for CH3/CH9). | **Turns off MOSFET pin (`HIGH`)**, forces status to 0, latches channel in `fault_manager`, and rejects CAN overrides. | Requires vehicle power cycle or authorized reset. | IDs `0x001`/`0x002` (Status=0), ID `0x501` (DTC $10+i$, Locked Mask) |
| **PDM** | **`100`** | `FAULT_CODE_VBAT_UNDERVOLTAGE` | `HW` | `HIGH` | Battery voltage $V_{\text{bat}} < 5.0\text{V}$ for $> 200\text{ ms}$. | **Cuts power to all 12 MOSFET channels** to protect battery cells against destructive deep discharge. | Automatic when $V_{\text{bat}} > 5.0\text{V}$ on power-up. | IDs `0x001`/`0x002` (All 0), ID `0x006` ($V_{\text{bat}}$), ID `0x501` (DTC `100`) |
| **PDM** | **`1`** | `FAULT_CODE_CAN_ERROR_PASSIVE` | `COMM` | `LOW` | Alert `ERR_PASS` due to bus degradation. | Internal diagnostic logging and telemetry warning. | Automatic when error rate decreases. | ID `0x501` (DTC `1`) |
| **PDM** | **`2`** | `FAULT_CODE_CAN_BUS_OFF` | `COMM` | `HIGH` | CAN bus error count $> 50$ or `BUS_OFF`. | Initiates automatic hardware CAN recovery cycle. | Automatic upon bus resynchronization. | ID `0x501` upon recovery |
| **MCU** | **`101`** | `FAULT_CODE_APPS_IMPLAUSIBLE` | `HW` | `HIGH` | Discrepancy between APPS1 and APPS2 $> 10\%$ for $> 100\text{ ms}$. | **Immediately clamps torque to $0.0\text{ Nm}$**, latches APPS subsystem, and disables inverter command. | Automatic when sensor discrepancy drops below $10\%$. | ID `0x502` (DTC `101`, APPS Mask Locked) |
| **MCU** | **`102`** | `FAULT_CODE_APPS_WIRE_BREAK` | `HW` | `HIGH` | Analog APPS voltage outside $\pm 15\%$ calibrated bounds. | **Immediately clamps torque to $0.0\text{ Nm}$** and blocks drive. | Restoration of valid analog voltage range. | ID `0x502` (DTC `102`) |
| **MCU** | **`103`** | `FAULT_CODE_BRAKE_SENSOR_ERR` | `HW` | `HIGH` | Hydraulic pressure sensor disconnected or out of electrical bounds ($< 50$ or $> 4000$). | Latches brake subsystem, blocks R2D entry, limits torque. | Restoration of valid electrical reading. | ID `0x502` (DTC `103`, Brakes Mask Locked) |
| **MCU** | **`104`** | `FAULT_CODE_TWAI_BUS_OFF` | `COMM` | `HIGH` | Inverter CAN or Car CAN in `BUS_OFF` state. | Initiates recovery cycle and forces $0.0\text{ Nm}$ torque. | Automatic upon physical bus recovery. | ID `0x502` upon recovery |
| **MCU** | **`105`** | `FAULT_CODE_BMS_SAG_LIMIT` | `RES` | `LOW` | HV battery pack voltage drops near minimum threshold during acceleration. | Dynamic torque derating via internal resistance ($R_{\text{int}}$) and OCV estimator to prevent BMS shutdown. | Dynamic based on open-circuit voltage. | ID `0x502` (DTC `105`) |
| **MCU** | **`106`** | `FAULT_CODE_BSPD_TRIPPED` | `HW` | `HIGH` | Accelerator $> 25\%$ with brake engaged ($> 100$ ADC). | **Hard clamp of torque demand to $0.0\text{ Nm}$** (FS EV4.7). | **Latched**: Clears only when accelerator pedal is released to $< 5\%$. | ID `0x502` (DTC `106`) |

---

## 3. Global Vehicle CAN Bus Communication Matrix (500 kbps)

| CAN ID | Message Name | Sender | Receivers | DLC | Rate | Payload Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`0x001`** | `PDM_MOSFETS_1_8` | **PDM** | Dashboard, MCU, Telemetry | 8 | 10 Hz | Status of MOSFET channels 1 through 8 ($1 = \text{ON}, 0 = \text{OFF}$). |
| **`0x002`** | `PDM_MOSFETS_9_12` | **PDM** | Dashboard, MCU, Telemetry | 4 | 10 Hz | Status of MOSFET channels 9 through 12 ($1 = \text{ON}, 0 = \text{OFF}$). |
| **`0x003`** | `PDM_CURRENTS_0_3` | **PDM** | Telemetry, Data Logger, MCU | 8 | 10 Hz | Current measurements for channels 0..3 (LE uint16 mA). |
| **`0x004`** | `PDM_CURRENTS_4_7` | **PDM** | Telemetry, Data Logger, MCU | 8 | 10 Hz | Current measurements for channels 4..7 (LE uint16 mA). |
| **`0x005`** | `PDM_CURRENTS_8_11` | **PDM** | Telemetry, Data Logger, MCU | 8 | 10 Hz | Current measurements for channels 8..11 (LE uint16 mA). |
| **`0x006`** | `PDM_CURRENTS_HALL_VBAT` | **PDM** | Telemetry, Data Logger, MCU | 8 | 10 Hz | Hall SD (mA), Hall Fans (mA), LV Battery Voltage (mV), Steering Wheel Alert. |
| **`0x020`** | `MCU_WHEEL_SPEEDS` | **MCU** | Dashboard, Telemetry, Logger | 8 | 100 Hz | Wheel speeds for FL, FR, RL, RR (BE uint16 RPM). |
| **`0x021`** | `MCU_VEHICLE_STATE` | **MCU** | Dashboard, ECU, PDM, Telemetry | 8 | 100 Hz | Steering angle, brake pressures, R2D state ($4 = \text{R2D}$), and demanded torque. |
| **`0x0C0`** | `INVERTER_TORQUE_CMD` | **MCU** | Unitek Bamocar Inverter | 8 | 100 Hz | Torque command register `0x90` and demanded value ($0 \dots 32767$). |
| **`0x100`** | `MANUAL_MOSFET_CMD` | **Steering Wheel** | PDM | 2 | On-Event | Channel ID ($0 \dots 11$) and power toggle command ($1 = \text{ON}, 0 = \text{OFF}$). |
| **`0x200`** | `MCU_LOG_TELEMETRY` | **MCU** | Data Logger, Telemetry | 8 | 10 Hz | Diagnostic logging and controller telemetry. |
| **`0x401`** | `ECU_TEMPS` | **ECU** | MCU, Dashboard, Telemetry | 4 | 1 Hz | Motor and Inverter temperatures (BE int16, $1^\circ\text{C}/\text{LSB}$). |
| **`0x402`** | `ECU_STS_GAUGES` | **ECU** | Telemetry, Dynamics Logger | 8 | 100 Hz | Raw 16-bit counts from 4 suspension travel strain gauges (RR, RL, FR, FL). |
| **`0x501`** | `PDM_DIAGNOSTIC_DTC` | **PDM** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | High Fault Active, Category, Priority, Active DTC, Locked Channels Bitmask. |
| **`0x502`** | `MCU_DIAGNOSTIC_DTC` | **MCU** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | High Fault Active, Category, Priority, Active DTC, Locked Subsystems Bitmask. |
| **`0x503`** | `ECU_DIAGNOSTIC_DTC` | **ECU** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | Thermal Failsafe Active, Active DTC, Motor Fan %, Inverter Fan %. |

---

## 4. Unified Unit Testing & CI/CD Verification Suite

All 15 test cases across the three firmware packages execute in native x86 environments and achieve 100% test pass rates in GitHub Actions CI:

```
--------------------------------------------------------------------------------
ECU_FW Unity Test Suite:
test/test_main.cpp:88: test_ads8688_bosch_ntc_conversion      [PASSED]
test/test_main.cpp:89: test_fan_driver_esc_scaling            [PASSED]
test/test_main.cpp:90: test_fan_driver_slew_rate              [PASSED]
test/test_main.cpp:91: test_pid_cooling_reverse_action        [PASSED]
test/test_main.cpp:92: test_fault_manager_failsafe_escalation [PASSED]
----------------------- 5 Tests 0 Failures 0 Ignored (PASSED) ------------------

PDM_FW Unity Test Suite:
test/test_main.cpp:95: test_vbat_conversion                   [PASSED]
test/test_main.cpp:96: test_protection_undervoltage_debounce  [PASSED]
test/test_main.cpp:97: test_protection_overcurrent_fast_trip  [PASSED]
test/test_main.cpp:98: test_protection_inrush_debouncing      [PASSED]
test/test_main.cpp:99: test_fault_manager_channel_lock        [PASSED]
----------------------- 5 Tests 0 Failures 0 Ignored (PASSED) ------------------

MCU_FW Unity Test Suite:
test/test_main.cpp:95: test_apps_calibration_and_deadband     [PASSED]
test/test_main.cpp:96: test_bspd_interlock                    [PASSED]
test/test_main.cpp:97: test_r2d_state_machine                 [PASSED]
test/test_main.cpp:98: test_torque_modes_and_limits           [PASSED]
test/test_main.cpp:99: test_fault_manager_apps_implausibility [PASSED]
----------------------- 5 Tests 0 Failures 0 Ignored (PASSED) ------------------
```

---

## 5. Industrial Quality & Safety Certification

1. **Zero Dynamic Memory Allocation**: Prohibits heap allocation (`malloc`, `free`, `new`, `delete`) across all production firmware modules. All tasks, queues, and buffers are 100% statically allocated at compile time.
2. **Hard Real-Time FreeRTOS Determinism**: 100 Hz primary execution loops utilizing `vTaskDelayUntil` for zero-jitter timing.
3. **MISRA-C:2012 Automotive Compliance**: Strict integer typing, explicit arithmetic conversions, bounded arrays, and zero uninitialized variables.
4. **Zero Warnings Enforcement**: Continuous compilation under `-Wall -Wextra -Werror` in ESP-IDF v5 and native GCC environments.
