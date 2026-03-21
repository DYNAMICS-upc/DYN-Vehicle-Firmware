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
        
        uint8_t speed = (adc_filtered * 255) / 4095;
        fan_driver_set_speed(speed);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
