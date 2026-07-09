#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pines APPS según mcu.ino (L25-L26)
#define PIN_APPS1 15  // EXT1
#define PIN_APPS2 16  // EXT2

// Calibración exacta de pedales según mcu.ino (L149-L153)
#define APPS1_CAL_REPOSO 1210
#define APPS1_CAL_FONDO  1830
#define APPS2_CAL_REPOSO 2492
#define APPS2_CAL_FONDO  1957  // Invertido (canal CCW de fábrica)
#define APPS_DEADBAND    14.0f // Zona muerta (%)

typedef struct {
    uint8_t apps1_pct;        // 0-100% para telemetría CAN
    uint8_t apps2_pct;        // 0-100% para telemetría CAN
    float throttle_cmd;       // 0-1000 resultado de control
    bool implausible_fault;   // Fallo de plausibilidad activo
    bool signal_cut;          // Par cortado a 0 por superar 100ms
} apps_data_t;

void apps_driver_init(uint8_t pin_main, uint8_t pin_sub);
bool apps_driver_read(uint16_t* out_throttle);
void apps_driver_get_telemetry(apps_data_t* out_data);

#ifdef __cplusplus
}
#endif
