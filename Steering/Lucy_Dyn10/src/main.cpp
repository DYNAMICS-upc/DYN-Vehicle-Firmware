#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include "button_driver.h"
#include "ipc.h"
#include "can_service.h"
#include "can_driver.h"
#include "led_driver.h"
#include "nextion_driver.h"
#include "buttons_app.h"

void can_task(void *pvParameters) {
    (void)pvParameters;
    can_service_init();
    while (1) {
        can_service_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void setup() {
    led_driver_init();
    nextion_driver_init();
    
    ipc_init();
    can_driver_init(10); // CS pin 10
    
    // Seed initial state
    volante_state_t init_state = {0};
    ipc_send_state(&init_state);

    buttons_app_init();
    
    // Fallback to dynamic allocation for AVR
    xTaskCreate(can_task, "CAN_Task", 256, NULL, 1, NULL);
}

void loop() {
    static uint32_t last_heartbeat = 0;
    if (millis() - last_heartbeat >= 500) {
        last_heartbeat = millis();
        led_driver_toggle(LED_HEARTBEAT);
    }
    
    volante_state_t state;
    if (ipc_peek_state(&state)) {
        led_driver_set(LED_R2D, state.dash.is_r2d);
        led_driver_set(LED_FAULT, state.has_bms_fault || state.has_imd_fault || state.dash.inv_fault);
        
        static uint8_t last_precharge_state = 255;
        if (state.dash.precharge_state != last_precharge_state) {
            last_precharge_state = state.dash.precharge_state;
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "tPrecharge.txt=\"State: %d\"", state.dash.precharge_state);
            nextion_driver_send_cmd(cmd);
        }
    }
    
    buttons_app_update();
    delay(10);
}
