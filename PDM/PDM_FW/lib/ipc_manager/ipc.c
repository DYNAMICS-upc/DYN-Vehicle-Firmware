#include "ipc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(mosfet_cmd_t)

static StaticQueue_t s_mosfet_queue_struct;
static uint8_t s_mosfet_queue_storage[QUEUE_LENGTH * ITEM_SIZE];
static QueueHandle_t s_mosfet_queue;

void ipc_init(void) {
    s_mosfet_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_mosfet_queue_storage, &s_mosfet_queue_struct);
}

bool ipc_send_mosfet_cmd(const mosfet_cmd_t* cmd) {
    if (s_mosfet_queue) {
        return xQueueSend(s_mosfet_queue, cmd, 0) == pdTRUE;
    }
    return false;
}

bool ipc_receive_mosfet_cmd(mosfet_cmd_t* cmd) {
    if (s_mosfet_queue) {
        return xQueueReceive(s_mosfet_queue, cmd, 0) == pdTRUE;
    }
    return false;
}
