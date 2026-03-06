#include <Arduino.h>
#include "fan_driver.h"

const int FAN_PIN = 9;
const int ADC_PIN = A0;

void setup() {
    fan_driver_init(FAN_PIN);
}

void loop() {
    int sensorValue = analogRead(ADC_PIN);
    uint8_t speed = (uint8_t)(sensorValue / 4); // Scale 0-1023 to 0-255
    fan_driver_set_speed(speed);
    delay(100);
}
