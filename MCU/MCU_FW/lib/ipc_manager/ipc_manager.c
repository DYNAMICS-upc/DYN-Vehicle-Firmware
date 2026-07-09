#include "ipc_manager.h"

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static StaticQueue_t s_can_tx_queue_struct;
static uint8_t s_can_tx_queue_storage[20 * sizeof(mcu_can_msg_t)];
static QueueHandle_t s_can_tx_queue;

void ipc_manager_init(void) {
    s_can_tx_queue = xQueueCreateStatic(20, sizeof(mcu_can_msg_t), s_can_tx_queue_storage, &s_can_tx_queue_struct);
}

bool ipc_manager_send_can_msg(const mcu_can_msg_t* msg) {
    if (!s_can_tx_queue) return false;
    return xQueueSend(s_can_tx_queue, msg, pdMS_TO_TICKS(10)) == pdTRUE;
}

bool ipc_manager_receive_can_msg(mcu_can_msg_t* msg) {
    if (!s_can_tx_queue) return false;
    return xQueueReceive(s_can_tx_queue, msg, 0) == pdTRUE;
}
#else
void ipc_manager_init(void) {}
bool ipc_manager_send_can_msg(const mcu_can_msg_t* msg) { (void)msg; return true; }
bool ipc_manager_receive_can_msg(mcu_can_msg_t* msg) { (void)msg; return false; }
#endif
