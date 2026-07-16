#include "can_service.h"
#include "fault_manager.h"
#include "ota_service.h"
#include "mosfet_driver.h"
#include "protection.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop
#include "esp_log.h"
static const char *TAG = "PDM_CAN";

static void can_rx_task(void *arg) {
    (void)arg;
    twai_message_t rx_msg = {};
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        while (twai_receive(&rx_msg, 0) == ESP_OK) {
            // ID 0x21: MCU Vehicle Status -> Check R2D in byte 6
            if (rx_msg.identifier == 0x21 && rx_msg.data_length_code >= 7) {
                bool is_r2d = (rx_msg.data[6] == 4);
                ota_set_r2d_state(is_r2d);
            }
            // ID 0x100: Comandos manuales de MOSFETs (con control de rechazo si el canal está en fallo)
            else if (rx_msg.identifier == 0x100 && rx_msg.data_length_code >= 2) {
                uint8_t ch = rx_msg.data[0];
                bool enable = (rx_msg.data[1] != 0);
                if (ch < MUX_CHANNELS) {
                    mosfet_driver_set_channel(ch, enable);
                }
            }
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10)); // 100 Hz
    }
}

void can_service_init(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        if (twai_start() == ESP_OK) {
            uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR;
            twai_reconfigure_alerts(alerts_to_enable, NULL);
        }
    }

    // Static FreeRTOS task pinned to Core 1
    static StaticTask_t s_rx_tcb;
    static StackType_t s_rx_stack[2048];
    xTaskCreateStaticPinnedToCore(can_rx_task, "pdm_can_rx", 2048, NULL, 9, s_rx_stack, &s_rx_tcb, 1);
}

void can_service_check_alerts(void) {
    uint32_t alerts_triggered = 0;
    if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(0)) == ESP_OK) {
        twai_status_info_t twaistatus;
        twai_get_status_info(&twaistatus);

        if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
            fault_manager_report(FAULT_CAT_COMMUNICATION, FAULT_PRIORITY_LOW, 1);
        }
        if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
            if (twaistatus.bus_error_count > 50) {
                fault_manager_report(FAULT_CAT_COMMUNICATION, FAULT_PRIORITY_HIGH, 2);
            }
        }
    }
}

void can_service_send_all_telemetry(const uint8_t *mosfets_status, const uint16_t *consumos_can, float v_bat_actual) {
    if (!mosfets_status || !consumos_can) return;

    twai_message_t msg;

    // ID 1: Mosfets 1-8
    memset(&msg, 0, sizeof(msg));
    msg.identifier = 1;
    msg.data_length_code = 8;
    msg.extd = 0;
    for (int i = 0; i < 8; i++) {
        msg.data[i] = mosfets_status[i];
    }
    twai_transmit(&msg, pdMS_TO_TICKS(10));

    // ID 2: Mosfets 9-12
    memset(&msg, 0, sizeof(msg));
    msg.identifier = 2;
    msg.data_length_code = 4;
    msg.extd = 0;
    for (int i = 0; i < 4; i++) {
        msg.data[i] = mosfets_status[8 + i];
    }
    twai_transmit(&msg, pdMS_TO_TICKS(10));

    // IDs 3, 4, 5, 6: Consumos + Voltaje Batería
    for (int id = 3; id <= 6; id++) {
        memset(&msg, 0, sizeof(msg));
        msg.identifier = id;
        msg.data_length_code = 8;
        msg.extd = 0;

        int startIdx = (id - 3) * 4;
        for (int i = 0; i < 4; i++) {
            if ((startIdx + i) < TOTAL_LOADS) {
                msg.data[i * 2] = consumos_can[startIdx + i] & 0xFF;
                msg.data[i * 2 + 1] = (consumos_can[startIdx + i] >> 8) & 0xFF;
            }
        }

        // ID 6: añadir voltaje batería en bytes 4 y 5, alerta de Volant en byte 6
        if (id == 6) {
            uint16_t vbat_can = (uint16_t)(v_bat_actual * 1000.0f);
            msg.data[4] = vbat_can & 0xFF;
            msg.data[5] = (vbat_can >> 8) & 0xFF;
            
            // Carga 4 (Volant+Dashes) corresponde al índice 3 (> 2500 mA)
            uint8_t alerta_carga4 = (consumos_can[3] > 2500) ? 1 : 0;
            msg.data[6] = alerta_carga4 & 0xFF;
            msg.data[7] = (uint8_t)(protection_get_warning_mask() & 0xFF); // Máscara de sobrecorriente (>110%)
        }

        twai_transmit(&msg, pdMS_TO_TICKS(10));
    }

    // Trama Diagnóstica Dedicada CAN ID 0x501 (DTC / Safe State)
    memset(&msg, 0, sizeof(msg));
    msg.identifier = CAN_ID_PDM_DIAGNOSTIC_DTC; // 0x501
    msg.data_length_code = 8;
    msg.extd = 0;
    fault_record_t rec = fault_manager_get_last_fault();
    msg.data[0] = fault_manager_is_high_fault_active() ? 1 : (rec.active ? 2 : 0);
    msg.data[1] = (uint8_t)rec.category;
    msg.data[2] = (uint8_t)rec.priority;
    msg.data[3] = (uint8_t)((rec.code >> 8) & 0xFF);
    msg.data[4] = (uint8_t)(rec.code & 0xFF);
    msg.data[5] = (uint8_t)((rec.fault_count >> 8) & 0xFF);
    msg.data[6] = (uint8_t)(rec.fault_count & 0xFF);
    msg.data[7] = (uint8_t)(fault_manager_get_locked_mask() & 0xFF);
    twai_transmit(&msg, pdMS_TO_TICKS(10));
}
#else
void can_service_init(void) {}
void can_service_check_alerts(void) {}
void can_service_send_all_telemetry(const uint8_t *mosfets_status, const uint16_t *consumos_can, float v_bat_actual) {
    (void)mosfets_status; (void)consumos_can; (void)v_bat_actual;
}
#endif
