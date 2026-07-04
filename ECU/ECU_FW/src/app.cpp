#include "app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fan_driver.h"
#include "ads8688_driver.h"
#include "bsp.h"
#include "ipc.h"
#include "ipc.h"
#include "can_service.h"
#include "fault_manager.h"

void app_init(void) {
    bsp_init();
    ipc_init();
    can_service_init();
    fault_manager_init();
    can_service_log("ECU INIT");
}

void app_run(void) {
    bool is_r2d = false; // Mock until CAN is fully integrated
    
    while (1) {
        uint16_t raw_temp = 0;
        if (!ads8688_driver_read_channel(0, &raw_temp)) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 1); // ADC Error
        } else if (fault_manager_is_high_fault_active()) {
            fault_manager_clear_all();
        }
        
        uint32_t adc_raw = raw_temp;
        // Simple Exponential Moving Average (EMA) filter to avoid floating noise
        static uint32_t s_adc_filtered = 0;
        s_adc_filtered = EMA_FILTER_SHIFT(s_adc_filtered, adc_raw, 3);
        
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
        
        if (fault_manager_is_high_fault_active()) {
            speed = 100; // Si hay fallo crítico, forzar ventiladores al 100% por seguridad
        }
        
        fan_driver_set_speed(speed);
        
        static uint32_t last_can_send = 0;
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (now - last_can_send >= 100) {
            last_can_send = now;
            QueueHandle_t txq = ipc_get_tx_queue();
            if (txq) {
                ecu_tx_frame_t frame = {};
                frame.id = ECU_CAN_ID_SENSORS;
                frame.dlc = 3;
                frame.data[0] = (s_adc_filtered >> 8) & 0xFF;
                frame.data[1] = s_adc_filtered & 0xFF;
                frame.data[2] = speed;
                xQueueSend(txq, &frame, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
