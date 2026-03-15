#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"

extern "C" void app_main(void) {
    mosfet_driver_init(18, 34); // Example ESP32 pins
    while (1) {
        mosfet_driver_set(true);
        if (mosfet_driver_check_fault()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
