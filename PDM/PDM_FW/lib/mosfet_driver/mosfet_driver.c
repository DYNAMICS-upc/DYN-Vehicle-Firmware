#include "mosfet_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
extern void digitalWrite(uint8_t pin, int val);
extern int analogRead(uint8_t pin);
#define LOW 0
#define HIGH 1
#endif

static uint8_t s_ctrl_pin = 0;
static uint8_t s_sense_pin = 0;
static const uint16_t MAX_CURRENT_THRESHOLD = 800; // Arbitrary ADC limit for mock

#ifndef ARDUINO
static uint16_t s_mock_current = 0;
void mosfet_driver_set_mock_current(uint16_t current_val) {
    s_mock_current = current_val;
}
int analogRead(uint8_t pin) {
    if (pin == s_sense_pin) return s_mock_current;
    return 0;
}
void digitalWrite(uint8_t pin, int val) {
    (void)pin;
    (void)val;
}
#endif

void mosfet_driver_init(uint8_t ctrl_pin, uint8_t sense_pin) {
    s_ctrl_pin = ctrl_pin;
    s_sense_pin = sense_pin;
}

void mosfet_driver_set(bool state) {
    digitalWrite(s_ctrl_pin, state ? HIGH : LOW);
}

bool mosfet_driver_check_fault(void) {
    int current = analogRead(s_sense_pin);
    if (current > MAX_CURRENT_THRESHOLD) {
        digitalWrite(s_ctrl_pin, LOW); // Force off on fault
        return true; // Fault detected
    }
    return false;
}
