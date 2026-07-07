#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pdm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void protection_init(void);

// Process the 10-sample loop over the 12 MUX channels and 2 Hall sensors
void protection_process_shunts_and_mux(uint16_t *out_consumos_can);

// Battery voltage measurement and 200ms debounce protection
bool protection_check_battery(float *out_vbat_actual, uint32_t current_time_ms);

// Direct single-sample check for tests and simulation
bool protection_check_channel_instant(uint8_t channel, float current_ma);

// Hall sensors processing
void protection_process_hall_sensors(uint16_t *out_consumos_can);

#ifdef __cplusplus
}
#endif
