# Technical Master Compliance & Safety Report: MCU (Motor Control Unit / VCU)

| Parameter | Specification |
| :--- | :--- |
| **Vehicle Subsystem** | Motor Control Unit (MCU / VCU - Central Vehicle Control Unit) |
| **Production Target** | [MCU_FW](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW) (ESP-IDF v5 + FreeRTOS SMP) |
| **Applied Standards** | MISRA-C:2012, ISO 26262 (ASIL-D / ASIL-B Principles), Zero Dynamic Memory, 100 Hz Deterministic FreeRTOS |
| **CI/CD Build & Test Status** | **PASSED** (Compilation with `-Werror`, static analysis, and 100% host unit tests passing) |

---

## 1. Executive Summary & System Architecture

The **Motor Control Unit (MCU / VCU)** is the central safety-critical supervisor and high-level vehicle controller of the Formula Student electric racing car:
1. **Redundant Dual Accelerator Pedal Position Sensing (APPS 1 & APPS 2)**: Median-of-3 filtering, wire-break and short-circuit boundary checking ($\pm 15\%$), continuous sensor plausibility checking ($< 10\%$ discrepancy) with a strict $100\text{ ms}$ grace window in accordance with Formula Student Electric vehicle regulations.
2. **Hydraulic Brake Pressure (HPS) & Dual-Segment Steering Sensing**: High-speed analog acquisition with calibrated deadbands and threshold triggers.
3. **Brake System Plausibility Device (BSPD) Interlock**: Hard torque cutoff to $0.0\text{ Nm}$ whenever accelerator demand exceeds $25\%$ while the mechanical brakes are actuated ($> 100$ ADC counts), latching until pedal demand drops below $5\%$.
4. **Ready-To-Drive (R2D) State Machine**: Secure multi-step arming sequence with Ready-To-Drive Sound (RTDS) acoustic horn timer ($2000\text{ ms}$).
5. **IRAM Hardware Interrupt Wheel Speed Processing**: Direct microsecond pulse period calculation across 4 phonic wheel Hall sensors (`ENC_FL`, `ENC_FR`, `ENC_RL`, `ENC_RR`) with $150\text{ ms}$ zero-speed timeout.
6. **Advanced Torque Management & Drive Modes**:
   - **Mode 1 (ECO / 40 kW)** and **Mode 2 (AutoX / 72 kW)** torque maps.
   - **Mode 3 (Endurance / 22 km)**: Closed-loop slow PI energy-budget controller with power slew rate limiter ($3\text{ kW/s}$), back-calculation anti-windup, and RPM-dependent power limits.
   - **Mode 8 (Launch Control)**: Target slip ratio regulation in static and dynamic phases.
   - **Regenerative Braking**: Fixed and brake-pressure proportional modes.
   - **Predictive Battery Sag Protection**: Adaptive internal resistance ($R_{\text{int}}$) and Open-Circuit Voltage (OCV) estimator preventing BMS undervoltage shutdowns.
7. **Dual-Bus CAN Architecture**:
   - **Inverter TWAI (500 kbps)**: High-speed $100\text{ Hz}$ cyclic torque commands (`REG_TORQUE = 0x90`, $0 \dots 32767$) to Unitek Bamocar.
   - **Car CAN (500 kbps)**: Real-time telemetry broadcasting (`0x020`, `0x021`, `0x200`) and DTC diagnostics on `0x502`.

---

## 2. Exhaustive Fault Management Matrix (DTC & Safe States)

The MCU firmware implements a dedicated fault manager (`fault_manager.c`) providing subsystem isolation, safe state latching, and CAN DTC broadcasting:

| DTC Code | Fault Identifier | Category | Priority | Exact Trigger Condition | System & Physical Hardware Reaction | Recovery / Reset Mechanism | Broadcasted CAN Frame |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`101`** | `FAULT_CODE_APPS_IMPLAUSIBLE` | `FAULT_CAT_HARDWARE` | `HIGH` | Discrepancy between APPS 1 and APPS 2 exceeds $10\%$ for $> 100\text{ ms}$ ($100,000\ \mu\text{s}$ grace period, FS Rules). | **Immediately clamps torque command to $0.0\text{ Nm}$**, disables inverter drive command, and latches APPS subsystem in `fault_manager_lock_subsystem(FAULT_SUBSYS_APPS)`. | Automatic recovery when the discrepancy between both sensor signals returns below $10\%$. | CAN ID `0x502`<br/>`Byte 0 = 1` (High Fault Active)<br/>`Byte 1 = 0` (HW)<br/>`Byte 2 = 1` (High Prio)<br/>`Bytes 3-4 = 101`<br/>`Byte 7 = Subsystems Bitmask` (Bit 0 = APPS) |
| **`102`** | `FAULT_CODE_APPS_WIRE_BREAK` | `FAULT_CAT_HARDWARE` | `HIGH` | Analog ADC voltage on APPS 1 or APPS 2 exceeds $\pm 15\%$ safety margins outside calibrated travel limits (indicates short to $3.3\text{V}$ or severed ground wire). | **Immediately clamps torque to $0.0\text{ Nm}$** and prohibits drive actuation. | Requires analog voltage to return within calibrated physical limits. | CAN ID `0x502`<br/>`Byte 0 = 1`<br/>`Bytes 3-4 = 102` |
| **`103`** | `FAULT_CODE_BRAKE_SENSOR_ERR` / `DISCONNECT` | `FAULT_CAT_HARDWARE` | `HIGH` | Hydraulic pressure sensor (HPS) disconnected or raw ADC outside valid electrical bounds ($< 50$ or $> 4000$ counts). | Latches brake subsystem mask (`FAULT_SUBSYS_BRAKES`), blocks entry into R2D driving state, and forces safe torque limiting. | Automatic recovery once valid electrical signal is re-established. | CAN ID `0x502`<br/>`Byte 0 = 1`<br/>`Bytes 3-4 = 103` |
| **`104`** | `FAULT_CODE_TWAI_BUS_OFF` | `FAULT_CAT_COMMUNICATION` | `HIGH` | Inverter CAN or Car CAN bus enters `BUS_OFF` state. | Initiates automatic hardware recovery via `twai_initiate_recovery()`, commands $0.0\text{ Nm}$ torque, and resets internal mailboxes. | Automatic once physical bus resynchronization is achieved. | CAN ID `0x502` immediately upon bus recovery |
| **`105`** | `FAULT_CODE_BMS_SAG_LIMIT` | `FAULT_CAT_RESOURCES` | `LOW` | High-voltage battery pack voltage drops near minimum discharge threshold during aggressive acceleration. | Adaptive torque derating based on estimated internal resistance $R_{\text{int}}$ and OCV to prevent tripping accumulator BMS shutdown. | Dynamic recovery as battery open-circuit voltage stabilizes. | CAN ID `0x502`<br/>`Byte 0 = 2` (Warning)<br/>`Bytes 3-4 = 105` |
| **`106`** | `FAULT_CODE_BSPD_TRIPPED` | `FAULT_CAT_HARDWARE` | `HIGH` | Accelerator pedal $> 25\%$ while hydraulic brake is actuated ($> 100$ ADC counts). | **Hard clamp of torque demand to $0.0\text{ Nm}$** (mandatory Formula Student EV rule EV4.7). | **Latched**: Does not clear until accelerator pedal is completely released to $< 5\%$. | CAN ID `0x502`<br/>`Byte 0 = 1`<br/>`Bytes 3-4 = 106` |

---

## 3. CAN / TWAI Bus Network Architecture (500 kbps)

### 3.1. Transmitted CAN Frames

| CAN ID | Message Name | DLC | Rate | Target Nodes | Payload Layout (Byte-by-Byte) | Resolution & Units |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`0x0C0`** | `INVERTER_TORQUE_CMD` | 8 | 100 Hz | Unitek Bamocar Motor Inverter | `Byte 0`: REG_TORQUE (`0x90`)<br/>`Byte 1-2`: Torque Demand (Little-Endian int16)<br/>`Byte 3..7`: Inverter Command Control | $32767 = 100\%$ nominal motor torque |
| **`0x020`** | `MCU_WHEEL_SPEEDS` | 8 | 100 Hz | Dashboard, Telemetry, Data Logger | `Byte 0-1`: RPM Front-Left (BE uint16)<br/>`Byte 2-3`: RPM Front-Right (BE uint16)<br/>`Byte 4-5`: RPM Rear-Left (BE uint16)<br/>`Byte 6-7`: RPM Rear-Right (BE uint16) | $1\ \text{LSB} = 1\ \text{RPM}$<br/>Range: $0 \dots 10000\ \text{RPM}$ |
| **`0x021`** | `MCU_VEHICLE_STATE` | 8 | 100 Hz | Dashboard, ECU, PDM, Telemetry | `Byte 0-1`: Steering Angle (BE int16, $0.1^\circ/\text{LSB}$)<br/>`Byte 2-3`: Front Brake Pressure (BE uint16)<br/>`Byte 4-5`: Rear Brake Pressure (BE uint16)<br/>`Byte 6`: Brake Switch & R2D State ($4 = \text{R2D}$)<br/>`Byte 7`: Demanded Torque Percentage ($0 \dots 100\%$) | Byte 6: Bit 0 = Brake Active, Bits 1-3 = R2D State |
| **`0x200`** | `MCU_LOG_TELEMETRY` | 8 | 10 Hz | Data Logger, Live Telemetry | Internal controller states and diagnostic logging | Structured telemetry payload |
| **`0x502`** | `MCU_DIAGNOSTIC_DTC` | 8 | 10 Hz / On-Fault | Safety Master, Dashboard, Telemetry | `Byte 0`: High Fault Active ($1 = \text{Yes}, 0 = \text{No}$)<br/>`Byte 1`: Category<br/>`Byte 2`: Priority<br/>`Byte 3-4`: Active DTC Code (Big-Endian uint16)<br/>`Byte 5-6`: Fault Count<br/>`Byte 7`: Locked Subsystems Bitmask | Bit 0: APPS, Bit 1: Brakes, Bit 2: Inverter, Bit 3: CAN Car |

### 3.2. Received CAN Frames

| CAN ID | Sender Node | Key Payload Fields | MCU Action & Functional Behavior |
| :--- | :--- | :--- | :--- |
| **`0x401`** | **ECU** (FANS_DYN10) | Motor & Inverter Temperatures ($^\circ\text{C}$). | Real-time thermal protection: triggers predictive torque derating if motor or inverter exceed critical operating temperature limits. |
| **`0x006`** | **PDM** | Auxiliary Battery Voltage & Current ($12.6\text{V}$). | Low-voltage system health monitoring. |
| **`0x180`** | **Inverter Bamocar** | Actual Motor Speed (RPM), Internal Torque, DC Bus Voltage. | Real-time feedback for traction control, launch control slip calculation, and regenerative power limits. |

---

## 4. Unit Test Validation Suite (Unity Test Framework)

The MCU firmware algorithms are thoroughly validated in native host simulation:

```
Processing * in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_main.cpp:95: test_apps_calibration_and_deadband     [PASSED]
test/test_main.cpp:96: test_bspd_interlock                    [PASSED]
test/test_main.cpp:97: test_r2d_state_machine                 [PASSED]
test/test_main.cpp:98: test_torque_modes_and_limits           [PASSED]
test/test_main.cpp:99: test_fault_manager_apps_implausibility [PASSED]
----------------------- 5 Tests 0 Failures 0 Ignored ---------------------------
OK
```

### Detailed Test Case Specifications

1. **`test_apps_calibration_and_deadband`**:
   - Asserts accelerator mapping across deadband ($14.0\%$), normal travel, and full-throttle points.
2. **`test_bspd_interlock`**:
   - Simulates brake pedal engagement during acceleration ($> 25\%$ APPS): confirms instantaneous torque cutoff to $0.0\text{ Nm}$ and verifies that torque cannot be restored until accelerator is fully released to $< 5\%$.
3. **`test_r2d_state_machine`**:
   - Tests transition sequence: `OFF` $\to$ `WAITING_BRAKE` $\to$ `WAITING_BUTTON` $\to$ `SOUNDING_RTDS` (verifies $2000\text{ ms}$ acoustic timer) $\to$ `READY_TO_DRIVE`.
4. **`test_torque_modes_and_limits`**:
   - Asserts maximum power and torque clipping for Mode 1 (ECO $40\text{ kW}$), Mode 2 (AutoX $72\text{ kW}$), and Launch Control slip regulation.
5. **`test_fault_manager_apps_implausibility`**:
   - Simulates sensor discrepancy $> 10\%$ for $> 100\text{ ms}$: validates generation of DTC `101`, activation of `FAULT_SUBSYS_APPS` lock, and complete torque nullification.

---

## 5. Industrial Standards & Safety Compliance Certification

1. **Zero Dynamic Memory Allocation**:
   - 100% static allocation. Zero use of dynamic heap (`malloc`, `free`, `new`).
   - Static FreeRTOS queues, semaphores, and tasks pinned to dedicated CPU cores.
2. **Deterministic Hard Real-Time Timing**:
   - Core control loop executes at strict $100\text{ Hz}$ ($10.0\text{ ms}$) via `vTaskDelayUntil`.
3. **MISRA-C:2012 Strict Conformance**:
   - Fully qualified integer types, bounded loop counters, explicit casts, and defensive programming throughout.
4. **Clean Compiler Verification**:
   - Zero warnings under `-Wall -Wextra -Werror` compiler flags.
