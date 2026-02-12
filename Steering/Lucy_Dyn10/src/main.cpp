#include <Arduino.h>

const int LED_PIN = 13;
const int BTN_PIN = 2;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
}

void loop() {
    int btn_state = digitalRead(BTN_PIN);
    if (btn_state == LOW) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
    delay(50);
}
