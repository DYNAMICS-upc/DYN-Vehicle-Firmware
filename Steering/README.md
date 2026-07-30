# Steering Wheel Firmware (Lucy_Dyn10)

## Overview
The Steering Wheel firmware serves as the primary human-machine interface (HMI) for the driver of the Dynamics UPC vehicle. It captures driver inputs (buttons, rotary switches, paddles) and displays critical vehicle telemetry on the dashboard in real-time, communicating seamlessly with the rest of the vehicle via the CAN network.

## Hardware Specifications
- **Microcontroller:** Arduino Mega (ATmega2560)
- **Framework:** Arduino
- **Architecture:** 8-bit AVR
- **Key Libraries:** `mcp_can` (CAN Communication), `FreeRTOS` (Task Management)

## Firmware Architecture
Despite using an 8-bit AVR microcontroller, the firmware is architected using a port of FreeRTOS. This allows for concurrent task execution, ensuring that the dashboard UI updates remain fluid while critical input polling and CAN message processing are handled deterministically.

### Key Responsibilities
- Polling steering wheel buttons, switches, and paddle shifters
- Formatting and transmitting driver inputs over CAN Bus
- Receiving and parsing vehicle telemetry (speed, temperatures, SoC, errors)
- Managing dashboard display updates and LED indicators

## Getting Started

### Prerequisites
- [PlatformIO IDE](https://platformio.org/)
- AVR device drivers

### Build & Flash
1. Navigate to the Steering firmware directory:
   ```bash
   cd Steering/Lucy_Dyn10
   ```
2. Build the project (PlatformIO will automatically fetch required libraries like `mcp_can` and `FreeRTOS`):
   ```bash
   pio run
   ```
3. Upload to the Arduino Mega:
   ```bash
   pio run -t upload
   ```
4. Open the Serial Monitor (115200 baud):
   ```bash
   pio device monitor
   ```

## Testing
The project supports native unit testing to validate logic independent of the AVR hardware.
Run tests via:
```bash
pio test -e native
```

## Contributing
When modifying HMI logic or display elements, ensure that tasks do not block the FreeRTOS scheduler. All CAN communication must adhere strictly to the vehicle's DBC definitions. Ensure zero warnings on build (`-Wall -Werror`).
