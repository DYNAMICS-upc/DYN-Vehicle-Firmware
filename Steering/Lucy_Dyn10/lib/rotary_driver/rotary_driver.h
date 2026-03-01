#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rotary_cb_t)(int delta);

void rotary_driver_init(uint8_t pin_a, uint8_t pin_b, rotary_cb_t on_change);
void rotary_driver_update(void);

// For testing purposes
void rotary_driver_set_pins(int state_a, int state_b);

#ifdef __cplusplus
}
#endif
