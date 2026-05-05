#include "can_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop

#define LOG_QUEUE_LEN 10

typedef struct {
    uint8_t data[8];
    uint8_t len;
} log_msg_t;

static StaticTask_t tx_task_tcb;
static StackType_t tx_task_stack[2048];
static QueueHandle_t s_log_queue;
static StaticQueue_t s_log_queue_struct;
static uint8_t s_log_queue_storage[LOG_QUEUE_LEN * sizeof(log_msg_t)];

static void can_tx_task(void *arg) {
    log_msg_t msg;
    while (1) {
        if (xQueueReceive(s_log_queue, &msg, portMAX_DELAY) == pdTRUE) {
            twai_message_t t_msg = {0};
            t_msg.identifier = 0x200; // Log ID
            t_msg.data_length_code = msg.len;
            memcpy(t_msg.data, msg.data, msg.len);
            twai_transmit(&t_msg, pdMS_TO_TICKS(5));
        }
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

    s_log_queue = xQueueCreateStatic(LOG_QUEUE_LEN, sizeof(log_msg_t), s_log_queue_storage, &s_log_queue_struct);
    xTaskCreateStaticPinnedToCore(can_tx_task, "can_tx", 2048, NULL, 8, tx_task_stack, &tx_task_tcb, 0);
}

void can_service_log(const char* str) {
    if (!s_log_queue || !str) return;
    log_msg_t msg;
    msg.len = strlen(str);
    if (msg.len > 8) msg.len = 8;
    memcpy(msg.data, str, msg.len);
    xQueueSend(s_log_queue, &msg, 0);
}
