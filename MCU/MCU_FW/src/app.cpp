#include "app.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "apps_driver.h"
#include "brake_driver.h"
#include "r2d_manager.h"
#include "shared_state.h"
#include "torque_ctrl.h"
#include "launch_ctrl.h"
#include "can_car_driver.h"
#include "ipc_manager.h"
#include "ota_service.h"
#include "fault_manager.h"

void app_init(void) {
    apps_driver_init(34, 35); // Example ESP32 ADC pins
    brake_driver_init(33); // Example Brake pin
    r2d_manager_init();
    shared_state_init();
    ipc_manager_init();
    torque_ctrl_init();
    launch_ctrl_init();
    can_car_init();
    fault_manager_init();
}

void app_run(void) {
    bool implausibility = false;

    while (1) {
        car_rx_data_t car_data;
        can_car_drain_rx(&car_data);
        
        uint16_t apps_val = 0;
        if (!apps_driver_read(&apps_val)) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 2); // Error 2: Sensor APPS desconectado o fuera de rango
        }

        uint16_t brake_val = 0;
        if (!brake_driver_read(&brake_val)) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 3); // Error 3: Sensor de freno desconectado
        }

        uint32_t speed_rpm = 0; // In a real scenario, this would come from the inverter/encoder

        // Lógica de implausibilidad temporal (Regla T.4.2 EV)
        // > 25% APPS (ej: >1000 raw) y freno pisado (ej: >500 raw)
        if (apps_val > 1000 && brake_val > 500) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 1); // Error 1: Implausibilidad Pedal
        }
        
        // Se resetea cuando APPS < 5% (ej: <200 raw)
        if (fault_manager_is_high_fault_active() && apps_val < 200) {
            fault_manager_clear_all();
        }

        mcu_shared_state_t state;
        shared_state_get(&state);
        
        state.ts_active = true; // dummy
        state.brake_pressed = brake_val > 500;
        state.r2d_button_pressed = (car_data.button_1 != 0);
        state.implausibility_fault = fault_manager_is_high_fault_active();
        
        r2d_manager_update(state.ts_active, state.brake_pressed, state.r2d_button_pressed);
        state.r2d_state = r2d_manager_get_state();
        
        shared_state_set(&state);

        bool r2d_active = (state.r2d_state == 4); // READY state
        ota_set_r2d_state(r2d_active);

        // Mock velocidades (luego vendrán del encoder y can)
        uint32_t speed_front = 50;
        uint32_t speed_rear = 55;
        
        int32_t slip_multiplier = launch_ctrl_update(speed_front, speed_rear);

        int32_t torque_command = torque_ctrl_calculate(apps_val, speed_rpm, state.brake_pressed, r2d_active, slip_multiplier);
        
        if (fault_manager_is_high_fault_active()) {
            torque_command = 0; // Corte inmediato y determinista por fallo High Priority
        }
        
        // torque_command would then be sent to the inverter via CAN

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
