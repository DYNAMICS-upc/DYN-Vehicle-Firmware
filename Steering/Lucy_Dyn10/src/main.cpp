#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include "button_driver.h"
#include "ipc.h"
#include "can_service.h"
#include "can_driver.h"

const int LED_PIN = 13;
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
    digitalWrite(LED_PIN, HIGH);
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = true;
        ipc_send_state(&state);
    }
}

void on_btn_release() {
    digitalWrite(LED_PIN, LOW);
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = false;
        ipc_send_state(&state);
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
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
    button_driver_update();
    delay(10);
}
