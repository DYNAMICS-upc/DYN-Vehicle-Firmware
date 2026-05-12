#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Límite de corriente para sobrecorriente
#define MUX_CHANNEL_MAX_CURRENT 800

void protection_init(void);
bool protection_check_mux_channel(uint8_t channel, uint16_t current_val);
bool protection_check_undervoltage(uint16_t vbat_mv);

#ifdef __cplusplus
}
#endif
