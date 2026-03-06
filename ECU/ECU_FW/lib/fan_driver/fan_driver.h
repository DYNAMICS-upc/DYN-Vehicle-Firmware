#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fan_driver_init(uint8_t pin);
void fan_driver_set_speed(uint8_t duty_cycle);

// Mock support for testing
#ifndef ARDUINO
uint8_t fan_driver_get_mock_speed(void);
#endif

#ifdef __cplusplus
}
#endif
