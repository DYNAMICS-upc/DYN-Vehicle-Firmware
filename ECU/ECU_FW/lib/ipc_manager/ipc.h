#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} ecu_tx_frame_t;

void ipc_init(void);
QueueHandle_t ipc_get_tx_queue(void);

#ifdef __cplusplus
}
#endif
