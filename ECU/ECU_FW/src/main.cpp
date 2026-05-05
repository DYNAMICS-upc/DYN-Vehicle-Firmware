#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fan_driver.h"
#include "pid_ctrl.h"
#include "ads8688_driver.h"
#include "bsp.h"
#include "can_service.h"

extern "C" void app_main(void) {
    bsp_init();
    can_service_init();
    
    can_service_log("ECU INIT");
    
    uint32_t adc_filtered = 0;
    bool is_r2d = false; // Mock until CAN is fully integrated
    
    while (1) {
        uint16_t raw_temp = 0;
        if (!ads8688_driver_read_channel(0, &raw_temp)) {
            // Error reading, fallback or keep old value
        }
        
        uint32_t adc_raw = raw_temp;
        // Simple Exponential Moving Average (EMA) filter to avoid floating noise
        static uint32_t s_adc_filtered = 0;
        s_adc_filtered = (s_adc_filtered * 7 + adc_raw) >> 3;
        
        // Mapa de temperaturas (LUT): Temp bruta -> PWM %
        uint8_t speed = 0;
        if (s_adc_filtered < 1000) {
            speed = 20; // 20%
        } else if (s_adc_filtered < 2000) {
            speed = 50; // 50%
        } else if (s_adc_filtered < 3000) {
            speed = 80; // 80%
        } else {
            speed = 100; // 100%
        }
        
        if (!is_r2d) {
            speed = 0; // Apagar ventiladores si no estamos en Ready to Drive
        }
        
        fan_driver_set_speed(speed);
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
