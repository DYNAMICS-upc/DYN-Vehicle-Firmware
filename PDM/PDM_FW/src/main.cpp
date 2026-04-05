#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"

extern "C" void app_main(void) {
    mosfet_driver_init(18, 34); // Example ESP32 pins
    while (1) {
        // Lógica combinacional para la bomba de agua
        bool inv_temp_high = false; // Dummy
        bool motor_temp_high = false; // Dummy
        bool manual_pump_override = true; // Dummy
        
        bool pump_enable = (inv_temp_high || motor_temp_high) || manual_pump_override;
        mosfet_driver_set(pump_enable);
        
        if (mosfet_driver_check_fault()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
