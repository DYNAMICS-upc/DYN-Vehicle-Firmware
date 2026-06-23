#include "can_car_driver.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TX_PIN 2
#define RX_PIN 42

static car_rx_data_t s_latest_data;

static void can_task(void *arg) {
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        twai_message_t rx_msg;
        while (twai_receive(&rx_msg, 0) == ESP_OK) {
            // Simulated decoding
            if (rx_msg.identifier == 0x100) {
                s_latest_data.timestamp_ms = xTaskGetTickCount();
            }
        }
        
        // Ensure 100 Hz deterministic loop for CAN processing
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}

void can_car_init(void) {
    memset(&s_latest_data, 0, sizeof(s_latest_data));
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); 
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        twai_start();
        // Pin task to Core 1 as claimed in TVSD
        xTaskCreatePinnedToCore(can_task, "can_task", 4096, NULL, 10, NULL, 1);
    }
}

bool can_car_rx_available(uint32_t wait_ms) {
    (void)wait_ms;
    return true;
}

void can_car_drain_rx(car_rx_data_t *out) {
    if (out) {
        *out = s_latest_data;
    }
}

void can_car_send_frame(const car_tx_frame_t *f) {
    twai_message_t tx_msg = {};
    tx_msg.identifier = f->id;
    tx_msg.data_length_code = f->dlc;
    memcpy(tx_msg.data, f->data, f->dlc);
    twai_transmit(&tx_msg, pdMS_TO_TICKS(1));
}

#else

static car_rx_data_t s_mock_data;

void can_car_init(void) {
    memset(&s_mock_data, 0, sizeof(s_mock_data));
}

bool can_car_rx_available(uint32_t wait_ms) {
    (void)wait_ms;
    return false;
}

void can_car_drain_rx(car_rx_data_t *out) {
    if (out) {
        *out = s_mock_data;
    }
}

void can_car_send_frame(const car_tx_frame_t *f) {
    (void)f;
}

#endif
