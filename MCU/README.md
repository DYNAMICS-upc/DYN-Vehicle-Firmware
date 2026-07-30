# Motor Control Unit (MCU) Firmware

## Overview
The Motor Control Unit (MCU) serves as the primary supervisory and powertrain controller for the Dynamics UPC vehicle. It manages the vehicle's state machine (such as Ready-to-Drive) and advanced vehicle dynamics (Launch Control, Torque Vectoring). It translates these high-level states into precise, low-level operational commands for the inverters and motors, ensuring maximum performance and safety.

## Hardware Specifications
- **Microcontroller:** ESP32 (esp32dev)
- **Framework:** ESP-IDF
- **Architecture:** 32-bit Xtensa dual-core
- **Communication:** CAN Bus (Inverter interface)

## Firmware Architecture
Built upon the ESP-IDF framework, the MCU firmware utilizes FreeRTOS to manage high-frequency control loops required for motor operations and vehicle dynamics algorithms. It is the central processing unit that dictates vehicle behavior and manages the physical drivetrain.

### Key Responsibilities
- Vehicle State Machine management (Ready-to-Drive, Fault states, etc.)
- Advanced vehicle dynamics (Launch Control, Traction Control, Torque Maps)
- Inverter communication and operational commands
- Powertrain limit enforcement and health monitoring
- Fault detection and emergency shutdown handling

## Getting Started

### Prerequisites
- [PlatformIO IDE](https://platformio.org/) installed
- Espressif 32 platform support

### Build & Flash
1. Navigate to the MCU firmware directory:
   ```bash
   cd MCU/MCU_FW
   ```
2. Compile the source code:
   ```bash
   pio run
   ```
3. Upload the firmware to the target device:
   ```bash
   pio run -t upload
   ```
4. Monitor serial output (115200 baud):
   ```bash
   pio device monitor
   ```

## Testing
Unit tests are implemented using the Unity testing framework for native execution.
To execute tests:
```bash
pio test -e native
```

## Contributing
Maintain strict adherence to the project coding standards. Ensure all CAN communication payloads are rigorously tested. Compilation must succeed without warnings (`-Wall -Werror`).
