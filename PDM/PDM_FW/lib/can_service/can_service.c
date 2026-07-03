#include "can_service.h"
#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop
#include "ipc.h"
#include "ota_service.h"

// Define CAN RX Task to read states and send them via IPC
static void can_rx_task(void *arg) {
    twai_message_t rx_msg;
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        while (twai_receive(&rx_msg, 0) == ESP_OK) {
            // Lectura de estados: por ejemplo, ID 0x100 es para comandos de MOSFET
            if (rx_msg.identifier == 0x100 && rx_msg.data_length_code >= 2) {
                mosfet_cmd_t cmd;
                cmd.mosfet_id = rx_msg.data[0];
                cmd.enable = (rx_msg.data[1] != 0);
                ipc_send_mosfet_cmd(&cmd);
            } else if (rx_msg.identifier == 0x401 && rx_msg.data_length_code >= 1) {
                // Mensaje ECU de estado de TS
                vehicle_state_t state;
                if (ipc_peek_vehicle_state(&state)) {
                    state.ts_active_req = (rx_msg.data[0] != 0);
                    ipc_send_vehicle_state(&state);
                }
            } else if (rx_msg.identifier == 0x200 && rx_msg.data_length_code >= 5) {
                // Mensaje inversor (simulado) para HV Voltage
                vehicle_state_t state;
                if (ipc_peek_vehicle_state(&state)) {
                    state.hv_voltage = (rx_msg.data[3] << 8) | rx_msg.data[4];
                    ipc_send_vehicle_state(&state);
                }
            } else if (rx_msg.identifier == 0x21 && rx_msg.data_length_code >= 7) {
                // Mensaje de estado R2D de la MCU
                ota_set_r2d_state(rx_msg.data[6] == 4);
            }
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10)); // 100 Hz deterministic loop
    }
}

void can_service_init(void) {
    // Basic TWAI config
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)39, (gpio_num_t)40, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        twai_start();
    }

    // Static task for CAN RX
    static StaticTask_t s_rx_tcb;
    static StackType_t s_rx_stack[2048];
    xTaskCreateStaticPinnedToCore(can_rx_task, "can_rx", 2048, NULL, 9, s_rx_stack, &s_rx_tcb, 1);
}

void can_service_send_precharge_state(uint8_t state) {
    twai_message_t tx_msg;
    tx_msg.identifier = 0x500; // PDM Precharge State ID
    tx_msg.extd = 0;
    tx_msg.rtr = 0;
    tx_msg.data_length_code = 1;
    tx_msg.data[0] = state;
    
    twai_transmit(&tx_msg, pdMS_TO_TICKS(10));
}
#else
void can_service_init(void) {}
void can_service_send_precharge_state(uint8_t state) { (void)state; }
#endif
