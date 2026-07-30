# Power Distribution Module (PDM) Firmware

## Overview
The Power Distribution Module (PDM) is the electrical heart of the Dynamics UPC vehicle. This firmware manages the intelligent distribution of low-voltage power to all vehicle subsystems, monitors current draw, handles electronic fusing (e-fuses), and protects the vehicle's electrical network from overcurrent conditions.

## Hardware Specifications
- **Microcontroller:** ESP32 (esp32dev)
- **Framework:** ESP-IDF
- **Architecture:** 32-bit Xtensa dual-core
- **Communication:** CAN Bus

## Firmware Architecture
The PDM firmware runs on ESP-IDF, providing a robust FreeRTOS foundation for continuous electrical monitoring and rapid fault response. It ensures that critical systems receive reliable power while isolating faults to prevent widespread system failure.

### Key Responsibilities
- Solid-state relay and e-fuse management
- Real-time current and voltage monitoring per channel
- Thermal monitoring of power distribution hardware
- Diagnostics reporting via CAN Bus
- Programmable trip currents and soft-start mechanisms

## Getting Started

### Prerequisites
- [PlatformIO IDE](https://platformio.org/)
- Applicable drivers for the ESP32 development board

### Build & Flash
1. Navigate to the PDM firmware directory:
   ```bash
   cd PDM/PDM_FW
   ```
2. Build the firmware image:
   ```bash
   pio run
   ```
3. Flash the image to the MCU:
   ```bash
   pio run -t upload
   ```
4. View real-time logs (115200 baud):
   ```bash
   pio device monitor
   ```

## Testing
Automated logic testing can be run natively utilizing the Unity framework.
To start the test suite:
```bash
pio test -e native
```

## Contributing
Due to the critical nature of power management, any changes to trip limits or switching logic must be thoroughly reviewed and tested. Verify that no compilation warnings exist (`-Werror` enabled).
