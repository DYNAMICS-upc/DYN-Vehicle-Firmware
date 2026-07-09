#include "can_car_driver.h"
#include "fault_manager.h"
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
    (void)arg;
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        twai_message_t rx_msg;
        while (twai_receive(&rx_msg, 0) == ESP_OK) {
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

void can_car_send_diagnostic_dtc(void) {
    fault_record_t rec = fault_manager_get_last_fault();
    car_tx_frame_t f = {};
    f.id = CAN_ID_MCU_DIAGNOSTIC_DTC; // 0x502
    f.dlc = 8;
    f.data[0] = fault_manager_is_high_fault_active() ? 1 : (rec.active ? 2 : 0);
    f.data[1] = (uint8_t)rec.category;
    f.data[2] = (uint8_t)rec.priority;
    f.data[3] = (uint8_t)((rec.code >> 8) & 0xFF);
    f.data[4] = (uint8_t)(rec.code & 0xFF);
    f.data[5] = (uint8_t)((rec.fault_count >> 8) & 0xFF);
    f.data[6] = (uint8_t)(rec.fault_count & 0xFF);
    f.data[7] = (uint8_t)(fault_manager_get_locked_subsystems() & 0xFF);
    
    can_car_send_frame(&f);
}

void can_car_send_inverter_torque(int16_t torque_nm) {
    car_tx_frame_t f = {};
    f.id = 0x0C0; // Inverter command ID
    f.dlc = 8;
    f.data[0] = (uint8_t)(torque_nm & 0xFF);
    f.data[1] = (uint8_t)((torque_nm >> 8) & 0xFF);
    can_car_send_frame(&f);
}

void can_car_send_wheel_speeds(uint16_t rpm_fl, uint16_t rpm_fr, uint16_t rpm_rl, uint16_t rpm_rr) {
    car_tx_frame_t f = {};
    f.id = 0x020; // Wheel Speeds CAN ID según mcu.ino L87, L517-L531
    f.dlc = 8;
    f.data[0] = (uint8_t)((rpm_fl >> 8) & 0xFF);
    f.data[1] = (uint8_t)(rpm_fl & 0xFF);
    f.data[2] = (uint8_t)((rpm_fr >> 8) & 0xFF);
    f.data[3] = (uint8_t)(rpm_fr & 0xFF);
    f.data[4] = (uint8_t)((rpm_rl >> 8) & 0xFF);
    f.data[5] = (uint8_t)(rpm_rl & 0xFF);
    f.data[6] = (uint8_t)((rpm_rr >> 8) & 0xFF);
    f.data[7] = (uint8_t)(rpm_rr & 0xFF);
    can_car_send_frame(&f);
}

void can_car_send_telemetry_all(const car_rx_data_t *rx, uint16_t apps1, uint16_t apps2,
                               uint16_t brake_front, uint16_t brake_rear,
                               int16_t steer, uint8_t r2d_state, int32_t torque_cmd) {
    (void)rx;
    (void)apps2;
    (void)brake_rear;
    car_tx_frame_t f = {};
    f.id = 0x021; // Sensors_Data CAN ID según mcu.ino L88, L533-L556
    f.dlc = 8;
    f.data[0] = (uint8_t)((steer >> 8) & 0xFF);
    f.data[1] = (uint8_t)(steer & 0xFF);
    f.data[2] = (uint8_t)((brake_front >> 8) & 0xFF);
    f.data[3] = (uint8_t)(brake_front & 0xFF);
    f.data[4] = 0; // HPS_R (anulado)
    f.data[5] = 0;
    f.data[6] = ((brake_front > 100) ? 1 : 0) | (r2d_state << 2);
    f.data[7] = (uint8_t)(torque_cmd & 0xFF);
    can_car_send_frame(&f);
}
#else
void can_car_init(void) {}
bool can_car_rx_available(uint32_t wait_ms) { (void)wait_ms; return false; }
void can_car_drain_rx(car_rx_data_t *out) { if (out) memset(out, 0, sizeof(*out)); }
void can_car_send_frame(const car_tx_frame_t *f) { (void)f; }
void can_car_send_diagnostic_dtc(void) {}
void can_car_send_inverter_torque(int16_t torque_nm) { (void)torque_nm; }
void can_car_send_wheel_speeds(uint16_t rpm_fl, uint16_t rpm_fr, uint16_t rpm_rl, uint16_t rpm_rr) {
    (void)rpm_fl; (void)rpm_fr; (void)rpm_rl; (void)rpm_rr;
}
void can_car_send_telemetry_all(const car_rx_data_t *rx, uint16_t apps1, uint16_t apps2,
                               uint16_t brake_front, uint16_t brake_rear,
                               int16_t steer, uint8_t r2d_state, int32_t torque_cmd) {
    (void)rx; (void)apps1; (void)apps2; (void)brake_front; (void)brake_rear;
    (void)steer; (void)r2d_state; (void)torque_cmd;
}
#endif
