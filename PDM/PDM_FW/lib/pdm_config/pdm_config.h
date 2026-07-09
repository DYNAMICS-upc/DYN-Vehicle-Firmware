#pragma once
#include <stdint.h>

// --- HARDWARE PINS (ESP32) ---
#define RX_PIN 40
#define TX_PIN 39

// Mux select pins (CD74HC4067 16-channel multiplexer)
#define MUX_PIN_S0 11
#define MUX_PIN_S1 12
#define MUX_PIN_S2 13
#define MUX_PIN_S3 14
#define MUX_COMMON_PIN 1  // ESP pin to read mux current measurements

// Voltage and Current Sensors
#define V_SENSE_PIN 2     // Battery voltage divider pin
#define HALL_SD_PIN 10    // Shutdown hall sensor pin (10A)
#define HALL_FANS_PIN 9   // Fans hall sensor pin (30A)

// Number of MUX channels used
#define MUX_CHANNELS 12
#define TOTAL_LOADS 14    // 12 MUX + 2 Hall sensors

// --- CALIBRATION & CONSTANTS ---
#define V_ESP 3.3f
#define ADC_MAX 4095.0f
#define SAMPLES_PER_LOOP 10
#define R_SHUNT 0.05f
#define GAIN 20.0f
#define ESCALA_CORRIENTE (1.0f / (GAIN * R_SHUNT)) // 1A = 1V

// Battery Voltage Divider: R2 = 4047.62, R3 = 1100.0
#define R2_DIV 4047.62f
#define R3_DIV 1100.0f

// Hall Sensors Sensitivity
#define SENS_SD 0.132f    // V/A
#define SENS_FANS 0.044f  // V/A
#define V_OFF_HALL 1.65f

// Channel indices
#define CANAL_VOLANT 3
#define CANAL_INVERTER 9

// Battery Undervoltage Limit
#define VBAT_MIN_LIMIT_V 5.0f
#define VBAT_UNDERVOLTAGE_DEBOUNCE_MS 200

// CAN Timing
#define CAN_INTERVAL_MS 100
