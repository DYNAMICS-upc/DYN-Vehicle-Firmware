#include "ipc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(mosfet_cmd_t)

static StaticQueue_t s_mosfet_queue_struct;
static uint8_t s_mosfet_queue_storage[QUEUE_LENGTH * ITEM_SIZE];
static QueueHandle_t s_mosfet_queue;
static QueueHandle_t s_vehicle_state_queue;

static StaticQueue_t s_vehicle_state_queue_struct;
static uint8_t s_vehicle_state_queue_storage[1 * sizeof(vehicle_state_t)];

void ipc_init(void) {
    s_mosfet_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_mosfet_queue_storage, &s_mosfet_queue_struct);
    s_vehicle_state_queue = xQueueCreateStatic(1, sizeof(vehicle_state_t), s_vehicle_state_queue_storage, &s_vehicle_state_queue_struct);

    vehicle_state_t init_state = {false, 0};
    xQueueSend(s_vehicle_state_queue, &init_state, 0);
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

bool ipc_send_vehicle_state(const vehicle_state_t* state) {
    if (s_vehicle_state_queue) {
        xQueueOverwrite(s_vehicle_state_queue, state);
        return true;
    }
    return false;
}

bool ipc_peek_vehicle_state(vehicle_state_t* state) {
    if (s_vehicle_state_queue) {
        return xQueuePeek(s_vehicle_state_queue, state, 0) == pdTRUE;
    }
    return false;
}
