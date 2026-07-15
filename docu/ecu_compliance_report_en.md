# Technical Master Compliance & Safety Report: ECU (FANS_DYN10)

| Parameter | Specification |
| :--- | :--- |
| **Vehicle Subsystem** | Electronic Control Unit (ECU - FANS_DYN10) |
| **Production Target** | [ECU_FW](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/ECU/ECU_FW) (ESP-IDF v5 + FreeRTOS SMP) |
| **Applied Standards** | MISRA-C:2012, ISO 26262 (ASIL-B Principles), Zero Dynamic Memory, 100 Hz Deterministic FreeRTOS |
| **CI/CD Build & Test Status** | **PASSED** (Compilation with `-Werror`, static analysis, and 100% host unit tests passing) |

---

## 1. Executive Summary & System Architecture

The **ECU (FANS_DYN10)** is an automotive embedded electronic control unit responsible for:
1. **High-Precision Thermal Acquisition**: Interfacing with the powertrain temperature sensors (Motor and Inverter) via the industrial 16-bit SPI **ADS8688** ADC and Bosch NTC thermistors.
2. **Closed-Loop Active Thermal Management (PID)**: Driving dual LEDC hardware PWM channels at 50 Hz with 14-bit resolution to command Electronic Speed Controllers (ESCs) for liquid/air cooling.
3. **High-Speed Suspension Strain Gauge Acquisition (100 Hz)**: Synchronously sampling 4 Suspension Travel Sensors (STS: Rear-Right, Rear-Left, Front-Right, Front-Left) for vehicle dynamics analysis.
4. **Deterministic CAN / TWAI Bus Networking (500 kbps)**: Broadcasting real-time telemetry (IDs `0x401` and `0x402`), handling diagnostic trouble codes (DTCs) on ID `0x503`, and receiving vehicle state frames (`0x021`) from the Motor Control Unit (MCU).
5. **Safety OTA Interlocking**: Hardware-level and software-level locking preventing remote firmware updates while the vehicle is in `READY_TO_DRIVE` state.

---

## 2. Exhaustive Fault Management Matrix (DTC & Safe States)

The ECU firmware implements an isolated, centralized fault manager (`fault_manager.c`) that classifies every anomaly, activates deterministic hardware failsafes, and broadcasts structured Diagnostic Trouble Codes (DTC) over the CAN bus.

| DTC Code | Fault Identifier | Category | Priority | Exact Trigger Condition | System & Physical Hardware Reaction | Recovery / Reset Mechanism | Broadcasted CAN Frame |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`201`** | `FAULT_CODE_MOTOR_NTC_FAIL` | `FAULT_CAT_HARDWARE` | `HIGH` | $\ge 3$ consecutive failed reads on Motor NTC channel (ADS8688 CH0 voltage outside $[0.01\text{V}, 5.11\text{V}]$ or computed temperature outside $[-40^\circ\text{C}, +130^\circ\text{C}]$). | Activates Thermal Failsafe State: Immediately commands Motor cooling fan to 10% duty and ramps up by +10% per second until reaching **100% continuous maximum cooling power**. Prevents motor thermal runaway in case of broken wire or loose sensor. | Automatic recovery when both Motor and Inverter NTC sensors provide valid temperatures simultaneously for 1 full second. | CAN ID `0x503`<br/>`Byte 0 = 1` (Failsafe Active)<br/>`Byte 1 = 0` (HW)<br/>`Byte 2 = 1` (High Prio)<br/>`Bytes 3-4 = 201`<br/>`Byte 5 = FanMotor%`<br/>`Byte 6 = FanInv%` |
| **`202`** | `FAULT_CODE_INV_NTC_FAIL` | `FAULT_CAT_HARDWARE` | `HIGH` | $\ge 3$ consecutive failed reads on Inverter NTC channel (ADS8688 CH7). | Activates Thermal Failsafe State: Ramps Inverter cooling fan to **100% maximum power** (+10%/s slew). Protects power semiconductors (IGBT/SiC modules) from unmonitored overheating. | Automatic recovery when both sensors return within valid calibration bounds. | CAN ID `0x503`<br/>`Byte 0 = 1`<br/>`Bytes 3-4 = 202` |
| **`203`** | `FAULT_CODE_ADS8688_SPI_ERR` | `FAULT_CAT_HARDWARE` | `HIGH` | SPI peripheral communication timeout or invalid NOP response from ADS8688 ADC chip. | Invalidates all temperature and suspension strain gauge telemetry. Forces both cooling fans to **100% continuous maximum duty**. | Requires SPI peripheral re-initialization or hard hardware reboot. | CAN ID `0x503`<br/>`Byte 0 = 1`<br/>`Bytes 3-4 = 203` |
| **`204`** | `FAULT_CODE_TWAI_BUS_OFF` | `FAULT_CAT_COMMUNICATION` | `HIGH` | TWAI/CAN peripheral enters `TWAI_STATE_BUS_OFF` due to physical bus degradation, missing termination resistors, or excessive frame collisions. | Invokes `twai_initiate_recovery()`, resets internal transmission buffers, and restarts CAN interface automatically. | Automatic once CAN controller completes the 128 occurrences of 11 consecutive recessive bits required by ISO 11898-1. | CAN ID `0x503` immediately upon bus recovery |

---

## 3. CAN / TWAI Bus Network Architecture (500 kbps)

All frames adhere to standard 11-bit CAN identifiers and are serialized with strict endianness and static memory buffers.

### 3.1. Transmitted CAN Frames

| CAN ID | Message Name | DLC | Rate | Target Nodes | Payload Layout (Byte-by-Byte) | Resolution & Units |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`0x401`** | `ECU_TEMPS` | 4 | 1 Hz | MCU, Dashboard, Data Logger, Telemetry | `Byte 0-1`: Motor Temperature (Big-Endian int16)<br/>`Byte 2-3`: Inverter Temperature (Big-Endian int16) | $1\ \text{LSB} = 1^\circ\text{C}$<br/>Range: $-40^\circ\text{C} \dots +130^\circ\text{C}$ |
| **`0x402`** | `ECU_STS_GAUGES` | 8 | 100 Hz | Telemetry, Vehicle Dynamics Logger | `Byte 0-1`: STS Rear-Right (Big-Endian uint16)<br/>`Byte 2-3`: STS Rear-Left (Big-Endian uint16)<br/>`Byte 4-5`: STS Front-Right (Big-Endian uint16)<br/>`Byte 6-7`: STS Front-Left (Big-Endian uint16) | $1\ \text{LSB} = 1\ \text{ADC Count}$<br/>Range: $0 \dots 65535$ (16-bit raw) |
| **`0x503`** | `ECU_DIAGNOSTIC_DTC` | 8 | 10 Hz / On-Fault | Safety Master, Dashboard, Telemetry | `Byte 0`: Failsafe Active ($1 = \text{Active}, 0 = \text{Normal}$)<br/>`Byte 1`: Category ($0=\text{HW}, 1=\text{Comm}, 2=\text{Res}, 3=\text{Timing}$)<br/>`Byte 2`: Priority ($0=\text{Low}, 1=\text{High}$)<br/>`Byte 3-4`: Active DTC Code (Big-Endian uint16)<br/>`Byte 5`: Motor Fan Duty ($0 \dots 100\%$) | `Byte 6`: Inverter Fan Duty ($0 \dots 100\%$)<br/>`Byte 7`: Cumulative Fault Count |

### 3.2. Received CAN Frames

| CAN ID | Sender Node | Key Payload Fields | ECU Action & Functional Behavior |
| :--- | :--- | :--- | :--- |
| **`0x021`** | **MCU** (Motor Control Unit) | `Byte 6`: Vehicle State & R2D ($4 = \text{READY\_TO\_DRIVE}$). | Interlocks the Over-The-Air (OTA) firmware update service (`ota_service.c`). Flashing is permanently rejected whenever the vehicle is operating or in ready-to-drive mode. |

---

## 4. Unit Test Validation Suite (Unity Test Framework)

All mathematical models, signal processing algorithms, and safety failsafes are validated in PlatformIO native x86 simulation using the **Unity** testing framework:

```
Processing * in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_main.cpp:88: test_ads8688_bosch_ntc_conversion      [PASSED]
test/test_main.cpp:89: test_fan_driver_esc_scaling            [PASSED]
test/test_main.cpp:90: test_fan_driver_slew_rate              [PASSED]
test/test_main.cpp:91: test_pid_cooling_reverse_action        [PASSED]
test/test_main.cpp:92: test_fault_manager_failsafe_escalation [PASSED]
----------------------- 5 Tests 0 Failures 0 Ignored ---------------------------
OK
```

### Detailed Test Case Specifications

1. **`test_ads8688_bosch_ntc_conversion`**:
   - Evaluates Steinhart-Hart / Bosch logarithmic interpolation table across calibrated operating points: verifies $20^\circ\text{C}$ at $2500\ \Omega$ and $50^\circ\text{C}$ at $834\ \Omega$.
   - Validates that out-of-range or negative resistance values reliably produce `NAN` to trigger fault handling.
2. **`test_fan_driver_esc_scaling`**:
   - Asserts pulse-width output: $0\% \to 1000\ \mu\text{s}$, $50\% \to 1570\ \mu\text{s}$, $100\% \to 2000\ \mu\text{s}$.
   - Verifies 14-bit timer duty calculation at 50 Hz ($T = 20000\ \mu\text{s}$), producing exact 819 integer duty counts for minimum $1000\ \mu\text{s}$ arming pulse.
3. **`test_fan_driver_slew_rate`**:
   - Checks rate-limiter slew dynamics ($20\%/\text{s}$): verifies that a $0 \to 100\%$ target step is clamped to $\le 2.0\%$ variation over a $100\text{ ms}$ interval.
4. **`test_pid_cooling_reverse_action`**:
   - Verifies reverse-acting PID control logic with a $45^\circ\text{C}$ setpoint: outputs $0.0\%$ duty below setpoint ($30^\circ\text{C}$) and delivers positive proportional-integral cooling duty when above setpoint ($55^\circ\text{C}$).
5. **`test_fault_manager_failsafe_escalation`**:
   - Verifies fault reporting, priority classification, failsafe flag latching, and DTC diagnostic record formatting.

---

## 5. Industrial Standards & Safety Compliance Certification

The ECU production codebase complies with standard automotive embedded requirements:

1. **Zero Dynamic Memory Allocation**:
   - No usage of `malloc`, `calloc`, `realloc`, `free`, `new`, or `delete`.
   - All FreeRTOS primitives (Tasks, Queues, Semaphores) instantiated via `xTaskCreateStaticPinnedToCore`, `xQueueCreateStatic`, and `xSemaphoreCreateMutexStatic`.
2. **Temporal Determinism & Multi-Core Execution**:
   - Core 0 runs deterministic $100\text{ Hz}$ analogue acquisition and $1\text{ Hz}$ PID thermal control with `vTaskDelayUntil`.
   - Core 1 handles asynchronous CAN transmission/reception with dedicated static queues (`ipc.c`), preventing latency spikes.
3. **MISRA-C:2012 Compliance**:
   - Strict explicit typing (`uint8_t`, `int16_t`, `uint32_t`, `float`, `double`).
   - All switch statements provide default handlers; no uninitialized variables or implicit sign conversions.
4. **Zero Compiler Warnings (`-Werror`)**:
   - Fully compliant build with `-Wall -Wextra -Werror -Wmissing-field-initializers` in ESP-IDF v5 and Native GCC.
