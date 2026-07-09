#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_ID_MCU_DIAGNOSTIC_DTC 0x502 // ID Diagnóstica segura no colisionante

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} car_tx_frame_t;

typedef struct {
    int16_t idc;
    uint8_t button_1;
    uint8_t button_launch;
    int16_t motor_rpm;
    uint8_t rotary_R1;
    uint8_t rotary_R2;
    uint8_t rotary_R3;
    int32_t Ax;
    int32_t Az;
    int32_t Vx;
    uint32_t timestamp_ms;
} car_rx_data_t;

void can_car_init(void);
bool can_car_rx_available(uint32_t wait_ms);
void can_car_drain_rx(car_rx_data_t *out);
void can_car_send_frame(const car_tx_frame_t *f);
void can_car_send_diagnostic_dtc(void);
void can_car_send_inverter_torque(int16_t torque_nm);
void can_car_send_wheel_speeds(uint16_t rpm_fl, uint16_t rpm_fr, uint16_t rpm_rl, uint16_t rpm_rr);
void can_car_send_telemetry_all(const car_rx_data_t *rx, uint16_t apps1, uint16_t apps2,
                               uint16_t brake_front, uint16_t brake_rear,
                               int16_t steer, uint8_t r2d_state, int32_t torque_cmd);

#ifdef __cplusplus
}
#endif
