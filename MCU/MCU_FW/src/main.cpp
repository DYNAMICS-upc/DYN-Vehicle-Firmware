#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "apps_driver.h"
#include "brake_driver.h"
#include "r2d_manager.h"

extern "C" void app_main(void) {
    apps_driver_init(34, 35); // Example ESP32 ADC pins
    brake_driver_init(33); // Example Brake pin
    r2d_manager_init();

    bool implausibility = false;

    while (1) {
        uint16_t apps_val = 0;
        uint16_t brake_val = 0;
        apps_driver_read(&apps_val);
        brake_driver_read(&brake_val);

        // Lógica de implausibilidad temporal (Regla T.4.2 EV)
        // > 25% APPS (ej: >1000 raw) y freno pisado (ej: >500 raw)
        if (apps_val > 1000 && brake_val > 500) {
            implausibility = true;
        }
        // Se resetea cuando APPS < 5% (ej: <200 raw)
        if (implausibility && apps_val < 200) {
            implausibility = false;
        }

        if (implausibility) {
            // Cortar par (dummy)
        }

        bool dummy_ts_active = true;
        bool dummy_button = false;
        r2d_manager_update(dummy_ts_active, brake_val > 500, dummy_button);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
