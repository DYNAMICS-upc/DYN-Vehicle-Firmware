#include "ipc.h"

#if defined(ESP_PLATFORM)
#define TX_QUEUE_LEN 32

static StaticQueue_t s_tx_queue_struct;
static uint8_t s_tx_queue_storage[TX_QUEUE_LEN * sizeof(ecu_tx_frame_t)];
static QueueHandle_t s_tx_queue = NULL;

void ipc_init(void) {
    s_tx_queue = xQueueCreateStatic(TX_QUEUE_LEN, sizeof(ecu_tx_frame_t), s_tx_queue_storage, &s_tx_queue_struct);
}

QueueHandle_t ipc_get_tx_queue(void) {
    return s_tx_queue;
}
#else
void ipc_init(void) {}
#endif
