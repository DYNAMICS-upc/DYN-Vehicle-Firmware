# Electronic Control Unit (ECU) Firmware

## Overview
The Electronic Control Unit (ECU) acts as the primary sensor acquisition node for the Dynamics UPC vehicle. It is strictly dedicated to reading data from various vehicle sensors, conditioning the signals, and reliably broadcasting this telemetry over the CAN network for use by other systems like the MCU.

## Hardware Specifications
- **Microcontroller:** ESP32 (esp32dev)
- **Framework:** ESP-IDF
- **Architecture:** 32-bit Xtensa dual-core
- **Communication:** CAN Bus, UART

## Firmware Architecture
The ECU firmware is developed using the official Espressif IoT Development Framework (ESP-IDF) to leverage a Real-Time Operating System (FreeRTOS) environment. This ensures deterministic behavior for critical control loops and vehicle safety states.

### Key Responsibilities
- Sensor data acquisition and signal conditioning
- Formatting and broadcasting sensor telemetry over CAN Bus
- High-frequency data polling and transmission
- System health and sensor redundancy checks

## Getting Started

### Prerequisites
- [PlatformIO IDE](https://platformio.org/) installed (VS Code extension recommended)
- Git for version control

### Build & Flash
1. Navigate to the ECU firmware directory:
   ```bash
   cd ECU/ECU_FW
   ```
2. Build the project:
   ```bash
   pio run
   ```
3. Flash to the ESP32 board (ensure the device is connected via USB):
   ```bash
   pio run --target upload
   ```
4. Open the Serial Monitor (115200 baud rate):
   ```bash
   pio device monitor
   ```

## Testing
The firmware includes a native testing environment using the Unity framework.
To run the native unit tests:
```bash
pio test -e native
```

## Contributing
Please ensure all code complies with the project's formatting guidelines. Any new feature should be accompanied by appropriate unit tests. Ensure zero compilation warnings (`-Werror` is enforced).
