#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*button_cb_t)(void);

void button_driver_init(uint8_t pin, button_cb_t on_press, button_cb_t on_release);
void button_driver_update(void);

#ifdef __cplusplus
}
#endif
