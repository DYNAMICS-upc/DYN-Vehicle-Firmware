#include "can_service.h"
#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "ipc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop

static StaticTask_t tx_task_tcb;
static StackType_t tx_task_stack[2048];

static void can_tx_task(void *arg) {
    ecu_tx_frame_t frame;
    QueueHandle_t txq = ipc_get_tx_queue();
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        if (xQueueReceive(txq, &frame, 0) == pdTRUE) {
            twai_message_t t_msg = {0};
            t_msg.identifier = frame.id;
            t_msg.data_length_code = frame.dlc;
            memcpy(t_msg.data, frame.data, frame.dlc);
            twai_transmit(&t_msg, 0);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10)); // 100 Hz deterministic loop
    }
}

void can_service_init(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)5, // TX mock
        (gpio_num_t)4, // RX mock
        TWAI_MODE_NORMAL
    );
    g_config.tx_queue_len = 10;
    
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        twai_start();
    }

    xTaskCreateStaticPinnedToCore(can_tx_task, "can_tx", 2048, NULL, 8, tx_task_stack, &tx_task_tcb, 1);
}

void can_service_log(const char* str) {
    QueueHandle_t txq = ipc_get_tx_queue();
    if (!txq || !str) return;
    ecu_tx_frame_t frame = {0};
    frame.id = 0x200; // Log ID
    frame.dlc = strlen(str);
    if (frame.dlc > 8) frame.dlc = 8;
    memcpy(frame.data, str, frame.dlc);
    xQueueSend(txq, &frame, 0);
}
#else
void can_service_init(void) {}
void can_service_log(const char* str) { (void)str; }
#endif
