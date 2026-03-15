#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "apps_driver.h"

extern "C" void app_main(void) {
    apps_driver_init(34, 35); // Example ESP32 ADC pins
    while (1) {
        uint16_t val;
        apps_driver_read(&val);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
