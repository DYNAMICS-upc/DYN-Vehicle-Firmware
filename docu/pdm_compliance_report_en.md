# Technical Master Compliance & Safety Report: PDM (Power Distribution Module)

| Parameter | Specification |
| :--- | :--- |
| **Vehicle Subsystem** | Power Distribution Module (PDM - Smart Solid-State LV Power Distribution) |
| **Production Target** | [PDM_FW](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/PDM/PDM_FW) (ESP-IDF v5 + FreeRTOS SMP) |
| **Applied Standards** | MISRA-C:2012, ISO 26262 (ASIL-B Principles), Zero Dynamic Memory, 100 Hz Deterministic FreeRTOS |
| **CI/CD Build & Test Status** | **PASSED** (Compilation with `-Werror`, static analysis, and 100% host unit tests passing) |

---

### 1. Executive Summary & System Architecture

The **Power Distribution Module (PDM)** serves as the intelligent solid-state electronic fuse box and power management hub for all Low-Voltage (LV) vehicle subsystems:
1. **Independent 12-Channel Solid-State MOSFET Power Switching**: Controls low-side/high-side power stages powering critical subsystems (Motor Control Unit, Inverter Logic, Cooling Pumps, Steering Wheel Electronics, Telemetry, Inertial Navigation SBG, ECU, etc.).
2. **Multi-Channel Precision Current Sensing**: Multiplexed acquisition across 12 resistive shunts ($R_{\text{shunt}} = 0.05\ \Omega$, operational amplifier gain $G = 20\ \text{V/V}$) via the **CD74HC4067** analog multiplexer, plus 2 dedicated high-current Hall-effect sensors (Shutdown Circuit 10A and Cooling Fans 30A).
3. **Automated 3-Tier Multi-Range Overcurrent Protection**:
   - **Tier 1 (Advisory Warning, $>110\% I_{\text{nom}}$)**: Broadcasts advisory alert, flags warning mask in telemetry (CAN ID 6 / 0x501), reports DTC `0x0200 + ch`; channel remains actively powered.
   - **Tier 2 (Timed Overload, $140\dots 170\% I_{\text{nom}}$)**: Initiates deterministic 60-second overload countdown timer (DTC `0x0300 + ch`); trips and permanently locks channel if overload is sustained for 60 seconds without recovering $<110\%$.
   - **Tier 3 (Instantaneous Trip, $>170\% I_{\text{nom}}$)**: Instant physical e-fuse trip ($<10\text{ ms}$), channel lockout, and DTC `0x0100 + ch` generation. (Channels 3 and 9 feature a 3-sample inrush filter).
4. **Low-Voltage Battery Undervoltage Protection**: Automatic global load shutdown if $V_{\text{bat}} < 5.0\text{V}$ persists for $> 200\text{ ms}$ (DTC `0x0199`), safeguarding battery chemistry against destructive deep discharge.
5. **Deterministic CAN / TWAI Telemetry (500 kbps)**: Emits complete switching state and milliampere current readings at 10 Hz across IDs `0x001` through `0x006`, alongside dedicated diagnostic trouble codes on ID `0x501`.
6. **Safety OTA Interlocking**: Rejects remote firmware updates whenever vehicle is in active `READY_TO_DRIVE` mode.

---

## 2. Exhaustive Fault Management Matrix (DTC & Safe States)

The PDM firmware features a safety-critical fault manager (`fault_manager.c`) with hardware latching and locked-channel tracking:

| DTC (Hex) | Fault Identifier | Category | Priority | Exact Trigger Condition | System & Physical Hardware Reaction | Recovery / Reset Mechanism | Broadcasted CAN Frame |
| :---: | :--- | :---: | :---: | :--- | :--- | :--- | :--- |
| **`0x0100` $\dots$ `0x010B`** | `FAULT_CODE_OVERCURRENT_CH0..11` | `FAULT_CAT_HARDWARE` | `HIGH` | Measured current on channel $i$ exceeds $>170\%$ of nominal limit (instant trip) or remains in $140\dots 170\%$ band for $\ge 60\text{s}$ (timed overload timeout). | Immediately turns off the MOSFET by setting gate pin to `HIGH` (OFF), sets `mosfets_status[i] = 0`, and engages channel latch via `fault_manager_lock_channel(i)`. **All subsequent CAN override reactivation commands (ID `0x100`) are rejected**. | Requires power cycling the vehicle or issuing an authorized diagnostic reset command once the overcurrent condition has been resolved. | CAN IDs `0x001`/`0x002` (Status byte = 0)<br/>CAN ID `0x501` (Byte 0 = 1, Bytes 3-4 = `0x0100 + i`, Byte 7 = Locked Channel Bitmask) |
| **`0x0199`** | `FAULT_CODE_VBAT_UNDERVOLTAGE` | `FAULT_CAT_HARDWARE` | `HIGH` | Low-voltage battery voltage $V_{\text{bat}} < 5.0\text{V}$ continuously for $> 200\text{ ms}$. | **Cuts power to all 12 MOSFET channels simultaneously** to protect the lithium auxiliary battery pack from catastrophic voltage collapse and cell degradation; locks all channels (`0x0FFF`). | Automatic recovery when battery voltage rises back above $5.0\text{V}$ during pre-charge or vehicle power-up. | CAN IDs `0x001`/`0x002` (All 12 bytes = 0)<br/>CAN ID `0x006` (Reports $V_{\text{bat}}$ in mV)<br/>CAN ID `0x501` (Byte 0 = 1, Bytes 3-4 = `0x0199`, Byte 7 = `0xFF`) |
| **`0x0200` $\dots$ `0x020B`** | `FAULT_CODE_WARN_OVERCURRENT_110_CH0..11` | `FAULT_CAT_HARDWARE` | `LOW` | Measured current on channel $i$ is within warning band ($110\% < I < 140\%$). | Sets bit $i$ in Warning Mask (CAN ID 6, Byte 7). MOSFET remains powered. | Clears automatically when current drops $\le 110\%$. | CAN ID `0x006` (Byte 7 warning mask)<br/>CAN ID `0x501` (Byte 0 = 2, Bytes 3-4 = `0x0200 + i`) |
| **`0x0300` $\dots$ `0x030B`** | `FAULT_CODE_WARN_OVERCURRENT_60S_CH0..11` | `FAULT_CAT_HARDWARE` | `LOW` | Measured current on channel $i$ is within overload band ($140\% \le I \le 170\%$). | Starts 60-second overload timer; sets bit $i$ in Timer Mask. MOSFET remains powered. | Clears automatically if current drops $\le 110\%$ before 60s timeout. | CAN ID `0x501` (Byte 0 = 2, Bytes 3-4 = `0x0300 + i`) |
| **`0x0401`** | `FAULT_CODE_CAN_PASSIVE_ERROR` | `FAULT_CAT_COMMUNICATION` | `LOW` | TWAI peripheral alert `TWAI_ALERT_ERR_PASS` triggered by elevated error counters on the physical bus. | Internal diagnostic logging; non-intrusive warning transmitted to telemetry logging station. | Automatic when communication error rate drops. | CAN ID `0x501` (Byte 0 = 2, Byte 2 = 0, Bytes 3-4 = `0x0401`) |
| **`0x0402`** | `FAULT_CODE_CAN_BUS_OFF` | `FAULT_CAT_COMMUNICATION` | `HIGH` | Controller enters `BUS_OFF` state or bus error count exceeds 50 (`bus_error_count > 50`). | Initiates automatic hardware recovery cycle via `twai_initiate_recovery()`. Retains all MOSFET safe states during bus reset. | Automatic once CAN controller successfully resynchronizes with the differential bus. | CAN ID `0x501` (Byte 0 = 1, Bytes 3-4 = `0x0402`) |

---

## 3. CAN / TWAI Bus Network Architecture (500 kbps)

### 3.1. Transmitted CAN Frames

| CAN ID | Message Name | DLC | Rate | Target Nodes | Payload Layout (Byte-by-Byte) | Resolution & Units |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`0x001`** | `PDM_MOSFETS_1_8` | 8 | 10 Hz | Dashboard, MCU, Telemetry | `Byte 0..7`: MOSFET 1 to 8 Status ($1 = \text{ON}, 0 = \text{OFF}$) | $1\ \text{Byte/Channel}$ (Boolean) |
| **`0x002`** | `PDM_MOSFETS_9_12` | 4 | 10 Hz | Dashboard, MCU, Telemetry | `Byte 0..3`: MOSFET 9 to 12 Status ($1 = \text{ON}, 0 = \text{OFF}$) | $1\ \text{Byte/Channel}$ (Boolean) |
| **`0x003`** | `PDM_CURRENTS_0_3` | 8 | 10 Hz | Telemetry, Data Logger, MCU | `Byte 0-1`: Current CH0 (Little-Endian uint16)<br/>`Byte 2-3`: Current CH1 (Little-Endian uint16)<br/>`Byte 4-5`: Current CH2 (Little-Endian uint16)<br/>`Byte 6-7`: Current CH3 (Little-Endian uint16) | $1\ \text{LSB} = 1\ \text{mA}$<br/>Range: $0 \dots 65535\ \text{mA}$ |
| **`0x004`** | `PDM_CURRENTS_4_7` | 8 | 10 Hz | Telemetry, Data Logger, MCU | `Byte 0-1`: Current CH4 (Little-Endian uint16)<br/>`Byte 2-3`: Current CH5 (Little-Endian uint16)<br/>`Byte 4-5`: Current CH6 (Little-Endian uint16)<br/>`Byte 6-7`: Current CH7 (Little-Endian uint16) | $1\ \text{LSB} = 1\ \text{mA}$ |
| **`0x005`** | `PDM_CURRENTS_8_11` | 8 | 10 Hz | Telemetry, Data Logger, MCU | `Byte 0-1`: Current CH8 (Little-Endian uint16)<br/>`Byte 2-3`: Current CH9 (Inverter) (LE uint16)<br/>`Byte 4-5`: Current CH10 (Little-Endian uint16)<br/>`Byte 6-7`: Current CH11 (Little-Endian uint16) | $1\ \text{LSB} = 1\ \text{mA}$ |
| **`0x006`** | `PDM_CURRENTS_HALL_VBAT` | 8 | 10 Hz | Telemetry, Data Logger, MCU | `Byte 0-1`: Hall Shutdown (LE uint16 mA)<br/>`Byte 2-3`: Hall Fans (LE uint16 mA)<br/>`Byte 4-5`: Battery Voltage LV (LE uint16 mV)<br/>`Byte 6`: Steering Wheel Warning ($1 = I_{\text{vol}} > 2.5\text{A}$)<br/>`Byte 7`: **Overcurrent Warning Mask ($>110\%$)** | Currents in $\text{mA}$, Voltage in $\text{mV}$ ($12600 = 12.6\text{V}$), Mask in Bitmask |
| **`0x501`** | `PDM_DIAGNOSTIC_DTC` | 8 | 10 Hz / On-Fault | Safety Master, Dashboard, Telemetry | `Byte 0`: High Fault Active ($1 = \text{Yes}, 0 = \text{No}$)<br/>`Byte 1`: Category<br/>`Byte 2`: Priority<br/>`Byte 3-4`: Active DTC Code (Big-Endian uint16)<br/>`Byte 5-6`: Fault Count<br/>`Byte 7`: Locked Channels Bitmask (Bit $i = \text{CH } i$) | Bit 0: CH0, Bit 1: CH1, ..., Bit 11: CH11 |

### 3.2. Received CAN Frames

| CAN ID | Sender Node | Key Payload Fields | PDM Action & Functional Behavior |
| :--- | :--- | :--- | :--- |
| **`0x021`** | **MCU** (Motor Control Unit) | `Byte 6`: Vehicle State & R2D ($4 = \text{READY\_TO\_DRIVE}$). | Interlocks the OTA firmware update service, forbidding remote code flashing during vehicle operation. |
| **`0x100`** | **Steering Wheel / Control** | `Byte 0`: Channel ID ($0 \dots 11$)<br/>`Byte 1`: Command ($1 = \text{Enable}, 0 = \text{Disable}$). | Toggles MOSFET power for the specified channel, strictly rejecting activation if the channel is latched in fault mode. |

---

## 4. Unit Test Validation Suite (Unity Test Framework)

All current monitoring equations, inrush filters, 3-tier protection ranges, and battery protection debouncers are verified in PlatformIO native x86 simulation:

```
Processing * in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_main.cpp:295: test_mosfet_init_and_control                                      [PASSED]
test/test_main.cpp:296: test_protection_range1_warning_above_110_percent                  [PASSED]
test/test_main.cpp:297: test_protection_range2_timer_start_between_140_and_170_percent     [PASSED]
test/test_main.cpp:298: test_protection_range2_timer_recovery_under_110_percent           [PASSED]
test/test_main.cpp:299: test_protection_range2_timer_expired_trips_and_locks_channel      [PASSED]
test/test_main.cpp:300: test_protection_range2_hysteresis_does_not_reset_if_above_110     [PASSED]
test/test_main.cpp:301: test_protection_range3_instant_trip_above_170_percent             [PASSED]
test/test_main.cpp:302: test_protection_check_channel_instant_wrapper                     [PASSED]
test/test_main.cpp:303: test_protection_warning_and_timer_masks                           [PASSED]
test/test_main.cpp:304: test_protection_inverter_persistence_3_samples                    [PASSED]
test/test_main.cpp:305: test_protection_volant_persistence_3_samples                      [PASSED]
test/test_main.cpp:306: test_protection_persistence_reset_on_recovery                     [PASSED]
test/test_main.cpp:307: test_protection_check_battery_undervoltage_debounce               [PASSED]
test/test_main.cpp:308: test_protection_process_shunts_and_mux_and_hall                   [PASSED]
test/test_main.cpp:309: test_dtc_error_codes_mapping                                      [PASSED]
test/test_main.cpp:310: test_fault_manager_records_and_clearing                           [PASSED]
----------------------- 16 Tests 0 Failures 0 Ignored ---------------------------
OK
```

### Detailed Test Case Specifications

1. **`test_mosfet_init_and_control`**:
   - Asserts individual MOSFET state setting, all-channel batch control (`mosfet_driver_set_all`), and validates that locked channels reject reactivation.
2. **`test_protection_range1_warning_above_110_percent`**:
   - Injects $115\% I_{\text{nom}}$ on Channel 0 ($2300\text{ mA}$ vs $2000\text{ mA}$ nominal): asserts `PROT_LEVEL_WARNING_110`, channel stays active (`mosfet == 1`), `warning_mask` bit 0 is set, and advisory DTC `0x0200` is generated with `FAULT_PRIORITY_LOW`.
3. **`test_protection_range2_timer_start_between_140_and_170_percent`**:
   - Injects $150\% I_{\text{nom}}$ ($3000\text{ mA}$): asserts `PROT_LEVEL_TIMER_ACTIVE`, starts 60s timer, channel stays active, `timer_mask` bit 0 is set, and DTC `0x0300` is broadcast.
4. **`test_protection_range2_timer_recovery_under_110_percent`**:
   - Injects $150\%$ at $t=1000\text{ ms}$ (timer starts), followed by current recovery to $105\%$ at $t=5000\text{ ms}$: asserts timer cancels, warning/timer masks clear, channel remains active.
5. **`test_protection_range2_timer_expired_trips_and_locks_channel`**:
   - Injects $150\%$ at $t=0\text{ ms}$, sustains overload through $t=60000\text{ ms}$ ($\Delta t = 60\text{s}$): asserts `PROT_LEVEL_TRIPPED`, MOSFET turned OFF (`0`), channel locked in `fault_manager`, and DTC `0x0100` reported with `FAULT_PRIORITY_HIGH`.
6. **`test_protection_range2_hysteresis_does_not_reset_if_above_110_percent`**:
   - Injects $150\%$ (starts timer), reduces current to $125\%$ ($>110\%$): asserts timer does NOT cancel, continuing toward trip unless dropped below $110\%$.
7. **`test_protection_range3_instant_trip_above_170_percent`**:
   - Injects $180\% I_{\text{nom}}$ on standard channels: asserts immediate cutoff ($<10\text{ ms}$), channel locked, MOSFET OFF, and DTC `0x0100 + ch` generated.
8. **`test_protection_inverter_persistence_3_samples` & `test_protection_volant_persistence_3_samples`**:
   - Verifies the 3-sample debouncing filter on capacitive/inductive channels (Inverter CH9 and Steering Wheel CH3): confirms that 1 or 2 high samples do not cause nuisance trips, and the channel trips reliably on the 3rd consecutive sample.
9. **`test_protection_check_battery_undervoltage_debounce`**:
   - Simulates a $< 5.0\text{V}$ voltage dip: verifies that transient drops lasting $< 200\text{ ms}$ are ignored, while conditions exceeding $200\text{ ms}$ trigger global channel shutdown, lock all channels, and trip DTC `0x0199`.
10. **`test_dtc_error_codes_mapping`**:
    - Validates exact DTC codes for Tier 1 warning (`0x0200 + ch`), Tier 2 timer start (`0x0300 + ch`), Tier 3 instant trip (`0x0100 + ch`), and Battery undervoltage (`0x0199`).
11. **`test_fault_manager_records_and_clearing`**:
    - Validates that tripped channels are latched in the fault manager bitmask, recorded with exact fault counts, and reject subsequent unauthorized reactivation commands.

---

## 5. Industrial Standards & Safety Compliance Certification

1. **Zero Dynamic Memory**:
   - 100% static allocation. Zero use of dynamic heap (`malloc`, `free`, `new`).
   - Static FreeRTOS queues and tasks pinned to Core 1 for telemetry serialization.
2. **Deterministic Hard Real-Time Timing**:
   - Primary protection loop executes at strict $100\text{ Hz}$ ($10.0\text{ ms}$) via `vTaskDelayUntil`.
3. **MISRA-C:2012 Strict Conformance**:
   - Fully qualified integer types, bounded loop counters, and explicit casts across all analog-to-digital conversions.
4. **Clean Compiler Verification**:
   - Zero warnings under `-Wall -Wextra -Werror` compiler flags.
