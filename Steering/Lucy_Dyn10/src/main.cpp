#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include "button_driver.h"
#include "ipc.h"
#include "can_service.h"
#include "can_driver.h"
#include "led_driver.h"

const int BTN_PIN = 2;

void can_task(void *pvParameters) {
    (void)pvParameters;
    can_service_init();
    while (1) {
        can_service_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void on_btn_press() {
    led_driver_set(LED_R2D, true);
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = true;
        ipc_send_state(&state);
    }
}

void on_btn_release() {
    led_driver_set(LED_R2D, false);
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = false;
        ipc_send_state(&state);
    }
}

void setup() {
    led_driver_init();
    pinMode(BTN_PIN, INPUT_PULLUP);
    
    ipc_init();
    can_driver_init(10); // CS pin 10
    
    // Seed initial state
    volante_state_t init_state = {0};
    ipc_send_state(&init_state);

    button_driver_init(BTN_PIN, on_btn_press, on_btn_release);
    
    // Fallback to dynamic allocation for AVR
    xTaskCreate(can_task, "CAN_Task", 256, NULL, 1, NULL);
}

void loop() {
    static uint32_t last_heartbeat = 0;
    if (millis() - last_heartbeat >= 500) {
        last_heartbeat = millis();
        led_driver_toggle(LED_HEARTBEAT);
    }
    button_driver_update();
    delay(10);
}
