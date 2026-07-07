#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pdm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void mosfet_driver_init(void);
void mosfet_driver_set_channel(uint8_t channel, bool enable);
void mosfet_driver_set_all(bool enable);
uint8_t mosfet_driver_get_status(uint8_t channel);
void mosfet_driver_get_all_statuses(uint8_t *out_statuses);

#ifdef __cplusplus
}
#endif
