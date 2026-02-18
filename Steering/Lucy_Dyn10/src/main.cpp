#include <Arduino.h>
#include <stdint.h>

const int LED_PIN = 13;
const int BTN_PIN = 2;

static uint32_t last_debounce_time = 0;
static const uint32_t DEBOUNCE_DELAY = 50;
static int last_btn_state = HIGH;
static int btn_state = HIGH;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
}

void loop() {
    int reading = digitalRead(BTN_PIN);

    if (reading != last_btn_state) {
        last_debounce_time = millis();
    }

    if ((millis() - last_debounce_time) > DEBOUNCE_DELAY) {
        if (reading != btn_state) {
            btn_state = reading;
            if (btn_state == LOW) {
                digitalWrite(LED_PIN, HIGH);
            } else {
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    last_btn_state = reading;
}
