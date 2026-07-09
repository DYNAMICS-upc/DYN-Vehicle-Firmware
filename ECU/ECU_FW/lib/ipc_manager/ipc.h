#pragma once
#include <stdint.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} ecu_tx_frame_t;

void ipc_init(void);
#if defined(ESP_PLATFORM)
QueueHandle_t ipc_get_tx_queue(void);
#endif

#ifdef __cplusplus
}
#endif
