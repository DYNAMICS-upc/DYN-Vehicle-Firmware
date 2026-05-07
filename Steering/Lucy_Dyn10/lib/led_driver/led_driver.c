#include "led_driver.h"
#include <Arduino.h>

static const uint8_t LED_PINS[LED_COUNT] = {
    8,  // LED_R2D
    9,  // LED_FAULT
    13  // LED_HEARTBEAT
};

void led_driver_init(void) {
    for (int i = 0; i < LED_COUNT; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        digitalWrite(LED_PINS[i], LOW);
    }
}

void led_driver_set(led_id_t led, bool state) {
    if (led < LED_COUNT) {
        digitalWrite(LED_PINS[led], state ? HIGH : LOW);
    }
}

void led_driver_toggle(led_id_t led) {
    if (led < LED_COUNT) {
        bool current = digitalRead(LED_PINS[led]);
        digitalWrite(LED_PINS[led], !current);
    }
}
