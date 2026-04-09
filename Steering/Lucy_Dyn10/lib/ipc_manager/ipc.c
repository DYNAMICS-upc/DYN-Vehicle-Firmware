#include "ipc.h"

// Golden Rule: NO dynamic memory allocation. Use StaticQueue_t.
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(volante_state_t)

static StaticQueue_t s_state_queue_cb;
static uint8_t s_state_queue_storage[QUEUE_LENGTH * ITEM_SIZE];

QueueHandle_t g_state_queue = NULL;

void ipc_init(void) {
    g_state_queue = xQueueCreateStatic(QUEUE_LENGTH,
                                       ITEM_SIZE,
                                       s_state_queue_storage,
                                       &s_state_queue_cb);
}

bool ipc_send_state(const volante_state_t* state) {
    if (g_state_queue == NULL || state == NULL) return false;
    return xQueueSend(g_state_queue, state, 0) == pdTRUE;
}

bool ipc_receive_state(volante_state_t* state) {
    if (g_state_queue == NULL || state == NULL) return false;
    return xQueueReceive(g_state_queue, state, 0) == pdTRUE;
}
