#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ads8688_driver_init(void);
bool ads8688_driver_read_channel(uint8_t channel, uint16_t* out_val);

#ifdef __cplusplus
}
#endif
