#include <Arduino.h>
#include "button_driver.h"

const int LED_PIN = 13;
const int BTN_PIN = 2;

void on_btn_press() {
    digitalWrite(LED_PIN, HIGH);
}

void on_btn_release() {
    digitalWrite(LED_PIN, LOW);
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    button_driver_init(BTN_PIN, on_btn_press, on_btn_release);
}

void loop() {
    button_driver_update();
}
