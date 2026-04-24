#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} car_tx_frame_t;

typedef struct {
    int16_t idc;
    uint8_t button_1;
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

#ifdef __cplusplus
}
#endif
