#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fan_driver.h"

extern "C" void app_main(void) {
    fan_driver_init(18);
    uint32_t adc_filtered = 0;
    while (1) {
        // Simulated ADC read (0-4095)
        uint32_t adc_raw = 2048; // Dummy for now
        // Simple Exponential Moving Average (EMA) filter to avoid floating noise
        adc_filtered = (adc_filtered * 7 + adc_raw) >> 3;
        
        // Basic PID implementation (integer only to respect rules)
        static int32_t error_integral = 0;
        static int32_t prev_error = 0;
        const int32_t target = 2000;
        
        int32_t error = target - (int32_t)adc_filtered;
        error_integral += error;
        int32_t error_deriv = error - prev_error;
        prev_error = error;
        
        // P=2, I=1, D=1 (dummy constants)
        int32_t output = (2 * error) + (1 * error_integral) + (1 * error_deriv);
        
        // Anti-windup & saturation
        if (output > 255) output = 255;
        if (output < 0) output = 0;
        
        uint8_t speed = (uint8_t)output;
        fan_driver_set_speed(speed);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
