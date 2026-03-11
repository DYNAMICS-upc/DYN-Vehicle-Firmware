#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fan_driver.h"

extern "C" void app_main(void) {
    fan_driver_init(18);
    while (1) {
        fan_driver_set_speed(128);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
