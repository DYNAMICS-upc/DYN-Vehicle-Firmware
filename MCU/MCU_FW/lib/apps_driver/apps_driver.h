#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void apps_driver_init(uint8_t pin_main, uint8_t pin_sub);
bool apps_driver_read(uint16_t* out_val);

// Mock support
#ifndef ARDUINO
void apps_driver_set_mock(uint16_t main_val, uint16_t sub_val);
#endif

#ifdef __cplusplus
}
#endif
