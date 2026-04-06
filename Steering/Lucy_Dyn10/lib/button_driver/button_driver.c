#include "button_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
extern int digitalRead(uint8_t pin);
extern uint32_t millis(void);
#define LOW 0
#define HIGH 1
#endif

static uint8_t s_pin = 0;
static button_cb_t s_on_press = NULL;
static button_cb_t s_on_release = NULL;

static uint32_t s_last_debounce_time = 0;
// 50ms de anti-rebote (debouncing)
static const uint32_t DEBOUNCE_DELAY = 50;
static int s_last_btn_state = HIGH;
static int s_btn_state = HIGH;

// Allow injecting time for testing
#ifndef ARDUINO
static uint32_t s_mock_time = 0;
void button_driver_set_time(uint32_t current_time_ms) {
    s_mock_time = current_time_ms;
}
static uint32_t get_time(void) { return s_mock_time; }
#else
static uint32_t get_time(void) { return millis(); }
#endif

void button_driver_init(uint8_t pin, button_cb_t on_press, button_cb_t on_release) {
    s_pin = pin;
    s_on_press = on_press;
    s_on_release = on_release;
    s_last_btn_state = HIGH;
    s_btn_state = HIGH;
}

void button_driver_update(void) {
    int reading = digitalRead(s_pin);
    uint32_t current_time = get_time();

    if (reading != s_last_btn_state) {
        s_last_debounce_time = current_time;
    }

    if ((current_time - s_last_debounce_time) > DEBOUNCE_DELAY) {
        if (reading != s_btn_state) {
            s_btn_state = reading;
            if (s_btn_state == LOW) {
                if (s_on_press != NULL) {
                    s_on_press();
                }
            } else {
                if (s_on_release != NULL) {
                    s_on_release();
                }
            }
        }
    }
    s_last_btn_state = reading;
}
