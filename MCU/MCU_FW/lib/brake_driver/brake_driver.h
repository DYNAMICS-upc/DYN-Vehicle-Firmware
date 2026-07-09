#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PIN_HPS_FRONT
#define PIN_HPS_FRONT 8
#endif
#ifndef PIN_HPS_REAR
#define PIN_HPS_REAR  18
#endif

void brake_driver_init(uint8_t pin);
bool brake_driver_read(uint16_t* out_raw);
uint8_t brake_driver_get_percentage(uint16_t raw_val);

#ifdef __cplusplus
}
#endif
