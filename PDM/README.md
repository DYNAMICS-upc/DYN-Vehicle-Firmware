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
- Solid-state relay and intelligent multi-tier e-fuse management:
  - **Tier 1 (Advisory Warning, $>110\% I_{\text{nom}}$)**: Sets warning bitmask in telemetry (CAN ID 6 / 0x501) and generates DTC `0x0200 + ch`; channel remains powered.
  - **Tier 2 (Timed Overload, $140\dots 170\% I_{\text{nom}}$)**: Starts 60-second overload countdown timer (DTC `0x0300 + ch`); trips and permanently locks channel if overload persists $\ge 60\text{s}$ without dropping $<110\%$.
  - **Tier 3 (Instant Cutoff, $>170\% I_{\text{nom}}$)**: Instant physical e-fuse trip ($<10\text{ ms}$), channel lockout, and DTC `0x0100 + ch` generation. (Includes 3-sample inrush filter on Inverter and Steering channels).
  - **Battery Undervoltage Protection**: 200 ms debounced global cutoff ($V_{\text{bat}} < 5.0\text{V}$, DTC `0x0199`) isolating all 12 MOSFET channels.
- Real-time current and voltage monitoring per channel (12 MUX channels + 2 Hall sensor high-current channels)
- Diagnostics reporting via CAN Bus (Dedicated 10 Hz frame CAN ID `0x501` + Telemetry IDs `0x001`..`0x006`)
- Safe state enforcement: Rejection of manual CAN turn-on commands on locked/faulted channels until formal diagnostic reset.

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
