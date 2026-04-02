#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void brake_driver_init(uint8_t pin);
bool brake_driver_read(uint16_t* out_val);

#ifdef __cplusplus
}
#endif
