#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include "volante_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// Expose the queue handle
extern QueueHandle_t g_state_queue;

void ipc_init(void);
bool ipc_send_state(const volante_state_t* state);
bool ipc_receive_state(volante_state_t* state);

#ifdef __cplusplus
}
#endif
