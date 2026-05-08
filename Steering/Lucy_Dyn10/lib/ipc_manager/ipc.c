#include "ipc.h"
#include <Arduino_FreeRTOS.h>
#include <queue.h>

// On AVR Arduino FreeRTOS, static allocation is disabled by default in FreeRTOSConfig.h.
// Falling back to dynamic allocation for xQueue.
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(volante_state_t)

static QueueHandle_t s_state_queue = NULL;

void ipc_init(void) {
    s_state_queue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
}

bool ipc_send_state(const volante_state_t* state) {
    if (s_state_queue == NULL || state == NULL) return false;
    return xQueueSend(s_state_queue, state, 0) == pdTRUE;
}

bool ipc_receive_state(volante_state_t* state) {
    if (s_state_queue == NULL || state == NULL) return false;
    return xQueueReceive(s_state_queue, state, 0) == pdTRUE;
}

bool ipc_peek_state(volante_state_t* state) {
    if (s_state_queue == NULL || state == NULL) return false;
    return xQueuePeek(s_state_queue, state, 0) == pdTRUE;
}
