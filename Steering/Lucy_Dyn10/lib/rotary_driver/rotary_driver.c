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
static int s_last_state_a = LOW;

#ifndef ARDUINO
static int mock_a = LOW;
static int mock_b = LOW;
void rotary_driver_set_pins(int state_a, int state_b) {
    mock_a = state_a;
    mock_b = state_b;
}
static int read_pin_a(void) { return mock_a; }
static int read_pin_b(void) { return mock_b; }
#else
static int read_pin_a(void) { return digitalRead(s_pin_a); }
static int read_pin_b(void) { return digitalRead(s_pin_b); }
#endif

void rotary_driver_init(uint8_t pin_a, uint8_t pin_b, rotary_cb_t on_change) {
    s_pin_a = pin_a;
    s_pin_b = pin_b;
    s_cb = on_change;
    s_last_state_a = read_pin_a();
}

void rotary_driver_update(void) {
    int state_a = read_pin_a();
    if (state_a != s_last_state_a) {
        if (read_pin_b() != state_a) {
            if (s_cb != NULL) { s_cb(1); }
        } else {
            if (s_cb != NULL) { s_cb(-1); }
        }
    }
    s_last_state_a = state_a;
}
