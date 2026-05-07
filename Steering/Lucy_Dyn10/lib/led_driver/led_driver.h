#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_R2D,
    LED_FAULT,
    LED_HEARTBEAT,
    LED_COUNT
} led_id_t;

void led_driver_init(void);
void led_driver_set(led_id_t led, bool state);
void led_driver_toggle(led_id_t led);

#ifdef __cplusplus
}
#endif
