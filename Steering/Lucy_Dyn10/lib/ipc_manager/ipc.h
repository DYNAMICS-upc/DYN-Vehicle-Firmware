#pragma once

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <stdbool.h>
#include "volante_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// The queue handle is now completely hidden inside ipc.c

void ipc_init(void);
bool ipc_send_state(const volante_state_t* state);
bool ipc_receive_state(volante_state_t* state);

#ifdef __cplusplus
}
#endif
