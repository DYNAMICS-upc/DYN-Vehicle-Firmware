#include "fan_driver.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
extern void analogWrite(uint8_t pin, int val);
#endif

static uint8_t s_pin = 0;

#ifndef ARDUINO
static uint8_t s_mock_speed = 0;
void analogWrite(uint8_t pin, int val) {
    (void)pin;
    s_mock_speed = (uint8_t)val;
}
uint8_t fan_driver_get_mock_speed(void) {
    return s_mock_speed;
}
#endif

void fan_driver_init(uint8_t pin) {
    s_pin = pin;
}

void fan_driver_set_speed(uint8_t duty_cycle) {
    // Basic scaling or raw duty cycle output
    analogWrite(s_pin, duty_cycle);
}
