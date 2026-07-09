#include "rotary_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
extern int digitalRead(uint8_t pin);
#define LOW 0
#define HIGH 1
#endif

static uint8_t s_pin_a = 0;
static uint8_t s_pin_b = 0;
static rotary_cb_t s_cb = NULL;
static int s_last_state_a = 0;

void rotary_driver_init(uint8_t pin_a, uint8_t pin_b, rotary_cb_t on_change) {
    s_pin_a = pin_a;
    s_pin_b = pin_b;
    s_cb = on_change;
    s_last_state_a = digitalRead(s_pin_a);
}

void rotary_driver_update(void) {
    int state_a = digitalRead(s_pin_a);
    if (state_a != s_last_state_a) {
        if (digitalRead(s_pin_b) != state_a) {
            if (s_cb != NULL) { s_cb(1); }
        } else {
            if (s_cb != NULL) { s_cb(-1); }
        }
    }
    s_last_state_a = state_a;
}
