#include <Arduino.h>
#include "mosfet_driver.h"

void setup() {
    mosfet_driver_init(5, A0);
}

void loop() {
    mosfet_driver_set(true);
    if (mosfet_driver_check_fault()) {
        // Handle fault
        delay(1000);
    }
    delay(10);
}
