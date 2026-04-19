#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "protection.h"

extern "C" void app_main(void) {
    mosfet_driver_init(18, 34); // Example ESP32 pins
    mux_adc_driver_init(25, 26, 27, 32); // Example S0, S1, S2, SIG pins
    protection_init();

    while (1) {
        // Ciclo de lectura de canales y protección
        bool safe = true;
        for (uint8_t i = 0; i < 8; i++) {
            mux_adc_driver_select(i);
            uint16_t adc_val = mux_adc_driver_read();
            if (!protection_check_mux_channel(i, adc_val)) {
                safe = false;
            }
        }

        // Lógica combinacional para la bomba de agua
        bool inv_temp_high = false; // Dummy
        bool motor_temp_high = false; // Dummy
        bool manual_pump_override = true; // Dummy
        
        bool pump_enable = (inv_temp_high || motor_temp_high) || manual_pump_override;
        
        if (!safe) {
            pump_enable = false; // Cut pump if not safe
        }

        mosfet_driver_set(pump_enable);
        mosfet_driver_update();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
