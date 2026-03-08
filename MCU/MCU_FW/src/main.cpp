#include <Arduino.h>
#include "apps_driver.h"

void setup() {
    apps_driver_init(A0, A1);
}

void loop() {
    uint16_t val;
    apps_driver_read(&val);
    delay(10);
}
