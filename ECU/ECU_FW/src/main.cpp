#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fan_driver.h"
#include "pid_ctrl.h"
#include "ads8688_driver.h"
#include "bsp.h"

extern "C" void app_main(void) {
    bsp_init();
    
    pid_ctrl_t fan_pid;
    pid_ctrl_init(&fan_pid, 2, 1, 1, 2000);
    
    uint32_t adc_filtered = 0;
    while (1) {
        uint16_t raw_temp = 0;
        if (!ads8688_driver_read_channel(0, &raw_temp)) {
            // Error reading, fallback or keep old value
        }
        
        uint32_t adc_raw = raw_temp;
        // Simple Exponential Moving Average (EMA) filter to avoid floating noise
        adc_filtered = (adc_filtered * 7 + adc_raw) >> 3;
        
        uint8_t speed = pid_ctrl_compute(&fan_pid, (int32_t)adc_filtered);
        fan_driver_set_speed(speed);
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
