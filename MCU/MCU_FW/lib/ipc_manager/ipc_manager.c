#include "ipc_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static QueueHandle_t s_can_tx_queue;

void ipc_manager_init(void) {
    s_can_tx_queue = xQueueCreate(20, sizeof(mcu_can_msg_t));
}

bool ipc_manager_send_can_msg(const mcu_can_msg_t* msg) {
    if (!s_can_tx_queue) return false;
    return xQueueSend(s_can_tx_queue, msg, pdMS_TO_TICKS(10)) == pdTRUE;
}

bool ipc_manager_receive_can_msg(mcu_can_msg_t* msg) {
    if (!s_can_tx_queue) return false;
    return xQueueReceive(s_can_tx_queue, msg, 0) == pdTRUE;
}
