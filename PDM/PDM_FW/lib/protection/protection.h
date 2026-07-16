#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pdm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PROT_LEVEL_NORMAL = 0,        // I <= 110% Inom (Normal operation)
    PROT_LEVEL_WARNING_110 = 1,   // 110% < I < 140% Inom (Warning alert message)
    PROT_LEVEL_TIMER_140_170 = 2, // 140% <= I <= 170% Inom (CAN alert + 60s timer active)
    PROT_LEVEL_TRIPPED_TIMED = 3, // 60s expired without dropping <= 110% (Cutoff & Locked)
    PROT_LEVEL_TRIPPED_INSTANT = 4// I > 170% Inom (Instant cutoff & Locked)
} protection_level_t;

void protection_init(void);

// Multi-tier timed current evaluation for a single channel
protection_level_t protection_check_channel(uint8_t channel, float current_ma, uint32_t current_time_ms);

// Single-sample check for instant overcurrent / backward compatibility (returns false if tripped)
bool protection_check_channel_instant(uint8_t channel, float current_ma);

// Status and telemetry query helpers
bool protection_is_warning_active(uint8_t channel);
bool protection_is_timer_running(uint8_t channel);
uint32_t protection_get_timer_elapsed_ms(uint8_t channel, uint32_t current_time_ms);
uint16_t protection_get_warning_mask(void);
uint16_t protection_get_timer_active_mask(void);

// Process the 10-sample loop over the 12 MUX channels and 2 Hall sensors with timestamp
void protection_process_shunts_and_mux(uint16_t *out_consumos_can, uint32_t current_time_ms);

// Battery voltage measurement and 200ms debounce protection
bool protection_check_battery(float *out_vbat_actual, uint32_t current_time_ms);

// Hall sensors processing
void protection_process_hall_sensors(uint16_t *out_consumos_can);

#ifdef __cplusplus
}
#endif

