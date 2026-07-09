#include "app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp.h"
#include "ipc.h"
#include "can_service.h"
#include "fault_manager.h"
#include "ads8688_driver.h"
#include "fan_driver.h"
#include "pid_ctrl.h"
#include "ota_service.h"

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#define SETPOINT_MOTOR_C 45.0
#define SETPOINT_INV_C   45.0
#define MAX_BAD_READS    3

static pid_ctrl_t s_pid_motor;
static pid_ctrl_t s_pid_inv;

static double s_temp_motor = 25.0;
static double s_temp_inv   = 25.0;

static double s_target_fan_motor = 0.0;
static double s_target_fan_inv   = 0.0;
static double s_current_fan_motor = 0.0;
static double s_current_fan_inv   = 0.0;

static uint8_t s_bad_reads_motor = 0;
static uint8_t s_bad_reads_inv   = 0;
static double  s_failsafe_pct    = 10.0;

void app_init(void) {
    bsp_init();
    ipc_init();
    can_service_init();
    fault_manager_init();
    ads8688_driver_init();
    fan_driver_init();

    // Inicializar PIDs de refrigeración (Acción reversa: Temp > 45°C -> sube ventilador)
    pid_ctrl_init(&s_pid_motor, 35.0, 0.08, 0.0, SETPOINT_MOTOR_C, PID_DIRECTION_REVERSE);
    pid_ctrl_init(&s_pid_inv,   35.0, 0.08, 0.0, SETPOINT_INV_C,   PID_DIRECTION_REVERSE);

    s_bad_reads_motor = 0;
    s_bad_reads_inv   = 0;
    s_failsafe_pct    = 10.0;

    can_service_log("ECU_INIT_OK");
}

void app_run(void) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t loop_count_10ms = 0;

    while (1) {
        loop_count_10ms++;

        // ==========================================================
        // 1. TAREA DE ALTA VELOCIDAD (100 Hz - 10 ms): Galgas STS
        // ==========================================================
        uint16_t raw_rr = 0, raw_rl = 0, raw_fr = 0, raw_fl = 0;
        ads8688_driver_read_raw(CH_STSRR, &raw_rr);
        ads8688_driver_read_raw(CH_STSRL, &raw_rl);
        ads8688_driver_read_raw(CH_STSFR, &raw_fr);
        ads8688_driver_read_raw(CH_STSFL, &raw_fl);

        // Envío determinista por CAN ID 11 (0x0B - STS) a 100 Hz (Big Endian idéntico a ecu.ino)
        QueueHandle_t txq = ipc_get_tx_queue();
        if (txq) {
            ecu_tx_frame_t frame_sts = {};
            frame_sts.id = CAN_ID_STS; // 11
            frame_sts.dlc = 8;
            frame_sts.data[0] = (uint8_t)(raw_rr >> 8);
            frame_sts.data[1] = (uint8_t)(raw_rr & 0xFF);
            frame_sts.data[2] = (uint8_t)(raw_rl >> 8);
            frame_sts.data[3] = (uint8_t)(raw_rl & 0xFF);
            frame_sts.data[4] = (uint8_t)(raw_fr >> 8);
            frame_sts.data[5] = (uint8_t)(raw_fr & 0xFF);
            frame_sts.data[6] = (uint8_t)(raw_fl >> 8);
            frame_sts.data[7] = (uint8_t)(raw_fl & 0xFF);
            xQueueSend(txq, &frame_sts, 0);
        }

        // ==========================================================
        // 2. TAREA DE REGULACIÓN TÉRMICA Y PID (1 Hz - cada 1000 ms)
        // ==========================================================
        if ((loop_count_10ms % 100) == 0) {
            double temp_read_m = 0.0;
            double temp_read_i = 0.0;
            bool ok_m = ads8688_driver_read_temp(CH_TEMP_MOTOR, &temp_read_m);
            bool ok_i = ads8688_driver_read_temp(CH_TEMP_INV, &temp_read_i);

            if (ok_m) {
                s_temp_motor = temp_read_m;
                s_bad_reads_motor = 0;
            } else {
                s_bad_reads_motor++;
            }

            if (ok_i) {
                s_temp_inv = temp_read_i;
                s_bad_reads_inv = 0;
            } else {
                s_bad_reads_inv++;
            }

            // Detección de fallo en sensores de temperatura
            bool failsafe_needed = false;
            if (s_bad_reads_motor >= MAX_BAD_READS) {
                fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_MOTOR_NTC_FAIL);
                failsafe_needed = true;
            }
            if (s_bad_reads_inv >= MAX_BAD_READS) {
                fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_INV_NTC_FAIL);
                failsafe_needed = true;
            }

            fault_manager_set_failsafe(failsafe_needed);

            if (fault_manager_is_failsafe_active()) {
                // Escalada de seguridad de ventilación: +10% cada segundo hasta el 100%
                s_failsafe_pct += 10.0;
                if (s_failsafe_pct > 100.0) s_failsafe_pct = 100.0;
                s_target_fan_motor = s_failsafe_pct;
                s_target_fan_inv   = s_failsafe_pct;
            } else {
                s_failsafe_pct = 10.0;
                s_target_fan_motor = pid_ctrl_compute(&s_pid_motor, s_temp_motor, 1.0);
                s_target_fan_inv   = pid_ctrl_compute(&s_pid_inv,   s_temp_inv,   1.0);
            }

            // Transmisión de Temperaturas en CAN ID 10 (0x0A) a 1 Hz (Big Endian idéntico a ecu.ino)
            if (txq) {
                int16_t t_m_int = (int16_t)lround(s_temp_motor);
                int16_t t_i_int = (int16_t)lround(s_temp_inv);

                ecu_tx_frame_t frame_temps = {};
                frame_temps.id = CAN_ID_TEMPS; // 10
                frame_temps.dlc = 4;
                frame_temps.data[0] = (uint8_t)(t_m_int >> 8);
                frame_temps.data[1] = (uint8_t)(t_m_int & 0xFF);
                frame_temps.data[2] = (uint8_t)(t_i_int >> 8);
                frame_temps.data[3] = (uint8_t)(t_i_int & 0xFF);
                xQueueSend(txq, &frame_temps, 0);
            }
        }

        // ==========================================================
        // 3. APLICACIÓN DE SLEW RATE LIMITER A LOS VENTILADORES (100 Hz)
        // ==========================================================
        s_current_fan_motor = fan_driver_slew_pct(s_current_fan_motor, s_target_fan_motor, 0.01);
        s_current_fan_inv   = fan_driver_slew_pct(s_current_fan_inv,   s_target_fan_inv,   0.01);

        fan_driver_set_speed(0, s_current_fan_motor);
        fan_driver_set_speed(1, s_current_fan_inv);

        // ==========================================================
        // 4. EMISIÓN DIAGNÓSTICA DTC (10 Hz - cada 100 ms)
        // ==========================================================
        if ((loop_count_10ms % 10) == 0) {
            can_service_send_diagnostic_dtc((uint8_t)s_current_fan_motor, (uint8_t)s_current_fan_inv);
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}
