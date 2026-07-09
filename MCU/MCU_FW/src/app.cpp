#include "app.h"
#include <stdio.h>
#include <string.h>

#include "bsp.h"
#include "apps_driver.h"
#include "brake_driver.h"
#include "encoder_driver.h"
#include "r2d_manager.h"
#include "shared_state.h"
#include "torque_ctrl.h"
#include "launch_ctrl.h"
#include "can_car_driver.h"
#include "ipc_manager.h"
#include "ota_service.h"
#include "fault_manager.h"

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

void app_init(void) {
    bsp_init();
    apps_driver_init(PIN_EXT1, PIN_EXT2);
    brake_driver_init(PIN_HPS_FRONT);
    encoder_driver_init();
    r2d_manager_init();
    shared_state_init();
    ipc_manager_init();
    torque_ctrl_init();
    launch_ctrl_init();
    can_car_init();
    fault_manager_init();
}

void app_run(void) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t loop_count_10ms = 0;

    while (1) {
        loop_count_10ms++;

        // 1. Drenar telemetría entrante de Inversor y Car CAN
        car_rx_data_t car_data = {};
        can_car_drain_rx(&car_data);
        
        // 2. Adquisición de Sensores APPS con comprobación de implausibilidad
        uint16_t apps_val = 0;
        if (!apps_driver_read(&apps_val)) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_APPS_IMPLAUSIBLE);
        }

        // 3. Adquisición de Sensores de Freno
        uint16_t brake_front_val = 0;
        uint16_t brake_rear_val = 0;
        if (!brake_driver_read(&brake_front_val)) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_BRAKE_DISCONNECT);
        }

        // 4. Adquisición de Encoders de Ruedas por interrupción hardware
        encoder_driver_update();
        wheel_speeds_t wheel_speeds = {};
        encoder_driver_get_speeds(&wheel_speeds);

        // 5. Adquisición de Ángulo de Dirección
        int16_t steering_angle = bsp_read_steering_angle();

        // 6. Lógica de Seguridad BSPD (Regla Formula Student EV: APPS > 25% con Freno accionado)
        bool brake_pressed = (brake_front_val > 500); // Umbral de presión de freno
        bsp_set_brake_light(brake_pressed);

        if (apps_val > 250 && brake_pressed) {
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_BSPD_TRIPPED);
        }
        
        // Reset de fallo de implausibilidad únicamente si el pedal retorna a < 5%
        if (fault_manager_is_high_fault_active() && apps_val < 50) {
            fault_manager_clear_all();
        }

        // 7. Gestión del Estado Ready to Drive (R2D)
        mcu_shared_state_t state;
        shared_state_get(&state);
        
        state.ts_active = bsp_read_ts_active();
        state.brake_pressed = brake_pressed;
        state.r2d_button_pressed = bsp_read_dash_button() || (car_data.button_launch != 0);
        state.implausibility_fault = fault_manager_is_high_fault_active();
        
        r2d_manager_update(state.ts_active, state.brake_pressed, state.r2d_button_pressed);
        state.r2d_state = r2d_manager_get_state();
        
        // Activación del Buzzer RTDS durante la fase de aviso acústico
        bsp_set_r2d_buzzer(state.r2d_state == R2D_STATE_SOUNDING);

        shared_state_set(&state);

        bool r2d_active = (state.r2d_state == R2D_STATE_READY);
        ota_set_r2d_state(r2d_active);

        // 8. Control de Lanzamiento / Deslizamiento dinámico
        int32_t slip_multiplier = launch_ctrl_update(wheel_speeds.speed_front_avg, wheel_speeds.speed_rear_avg);

        // 9. Cálculo de Par Motor con limitación de potencia y filtro anti-kick
        uint32_t speed_rpm = (car_data.motor_rpm > 0) ? (uint32_t)car_data.motor_rpm : 0;
        int32_t torque_command = torque_ctrl_calculate(apps_val, speed_rpm, state.brake_pressed, r2d_active, slip_multiplier);
        
        // Si hay fallo crítico o no estamos en R2D, el par se anula incondicionalmente
        if (fault_manager_is_high_fault_active() || !r2d_active) {
            torque_command = 0;
        }

        // 10. Transmisión del comando de Par al Inversor por TWAI (100 Hz)
        can_car_send_inverter_torque((int16_t)torque_command);

        // 11. Emisión periódica de Telemetría Car CAN (100 Hz)
        can_car_send_telemetry_all(&car_data, apps_val, apps_val, brake_front_val, brake_rear_val, 
                                   steering_angle, (uint8_t)state.r2d_state, torque_command);
        can_car_send_wheel_speeds((uint16_t)wheel_speeds.rpm_fl, (uint16_t)wheel_speeds.rpm_fr,
                                  (uint16_t)wheel_speeds.rpm_rl, (uint16_t)wheel_speeds.rpm_rr);

        // 12. Emisión de Trama Diagnóstica DTC en CAN ID 0x502 (10 Hz)
        if ((loop_count_10ms % 10) == 0) {
            can_car_send_diagnostic_dtc();
        }

        // Bucle determinista a 100 Hz (10 ms)
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}
