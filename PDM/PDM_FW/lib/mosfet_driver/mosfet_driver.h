#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mosfet_driver_init(uint8_t ctrl_pin, uint8_t sense_pin);
void mosfet_driver_set(bool state);
bool mosfet_driver_check_fault(void);
void mosfet_driver_update(void);

// Mock support
#ifndef ARDUINO
void mosfet_driver_set_mock_current(uint16_t current_val);
#endif

#ifdef __cplusplus
}
#endif
