#include "apps_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
extern int analogRead(uint8_t pin);
#endif

static uint8_t s_pin_main = 0;
static uint8_t s_pin_sub = 0;

#ifndef ARDUINO
static uint16_t s_mock_main = 0;
static uint16_t s_mock_sub = 0;
void apps_driver_set_mock(uint16_t main_val, uint16_t sub_val) {
    s_mock_main = main_val;
    s_mock_sub = sub_val;
}
int analogRead(uint8_t pin) {
    if (pin == s_pin_main) return s_mock_main;
    if (pin == s_pin_sub) return s_mock_sub;
    return 0;
}
#endif

void apps_driver_init(uint8_t pin_main, uint8_t pin_sub) {
    s_pin_main = pin_main;
    s_pin_sub = pin_sub;
}

bool apps_driver_read(uint16_t* out_val) {
    if (out_val == NULL) return false;
    
    int val_main = analogRead(s_pin_main);
    int val_sub = analogRead(s_pin_sub);
    
    // Plausibility check: main should be roughly double sub
    if (val_main > (val_sub * 2 + 100) || val_main < (val_sub * 2 - 100)) {
        return false;
    }
    
    *out_val = (uint16_t)val_main;
    return true;
}
