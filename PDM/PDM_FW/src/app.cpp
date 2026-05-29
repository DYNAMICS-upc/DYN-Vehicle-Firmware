#include "app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "protection.h"
#include "ipc.h"
#include "can_service.h"

// --- Hardware Pins ---
#define MOSFET_PIN_LATCH    18
#define MOSFET_PIN_ENABLE   34
#define MUX_PIN_S0          25
#define MUX_PIN_S1          26
#define MUX_PIN_S2          27
#define MUX_PIN_SIG         32

// --- System Constants ---
#define MUX_CHANNELS                  8
#define MOCK_VBAT_MV                  12500
#define PRECHARGE_HV_THRESHOLD_V      400
#define PRECHARGE_TIMEOUT_MS          3000

static inline bool check_precharge_timeout(uint32_t start_ms) {
    return ((xTaskGetTickCount() * portTICK_PERIOD_MS) - start_ms) > PRECHARGE_TIMEOUT_MS;
}

void app_init(void) {
    mosfet_driver_init(MOSFET_PIN_LATCH, MOSFET_PIN_ENABLE);
    mux_adc_driver_init(MUX_PIN_S0, MUX_PIN_S1, MUX_PIN_S2, MUX_PIN_SIG);
    protection_init();
    ipc_init();
    can_service_init();
}

void app_run(void) {
    while (1) {
        // Procesar comandos de MOSFET desde la cola IPC
        mosfet_cmd_t cmd;
        if (ipc_receive_mosfet_cmd(&cmd)) {
            // Ignoramos el mosfet_id para la simulación actual
            mosfet_driver_set(cmd.enable);
        }

        // Ciclo de lectura de canales y protección
        bool safe = true;
        for (uint8_t i = 0; i < MUX_CHANNELS; i++) {
            mux_adc_driver_select(i);
            uint16_t adc_val = mux_adc_driver_read();
            if (!protection_check_mux_channel(i, adc_val)) {
                safe = false;
            }
        }
        
        uint16_t vbat_val = MOCK_VBAT_MV;
        if (!protection_check_undervoltage(vbat_val)) {
            safe = false;
        }

        // Lógica combinacional para la bomba de agua
        bool inv_temp_high = false; // Dummy
        bool motor_temp_high = false; // Dummy
        bool manual_pump_override = true; // Dummy
        
        bool pump_enable = (inv_temp_high || motor_temp_high) || manual_pump_override;
        
        if (!safe) {
            pump_enable = false; // Cut pump if not safe
            // Override queue state if protection triggers
            mosfet_driver_set(false);
        }

        // Lógica de Precharge
        static enum {
            PRECHARGE_OFF,
            PRECHARGE_PRECHARGING,
            PRECHARGE_ON,
            PRECHARGE_ERROR
        } precharge_state = PRECHARGE_OFF;
        static uint32_t precharge_start = 0;
        
        vehicle_state_t v_state = {false, 0};
        ipc_peek_vehicle_state(&v_state);
        
        bool ts_active_req = v_state.ts_active_req;
        uint16_t hv_voltage = v_state.hv_voltage;
        
        switch (precharge_state) {
            case PRECHARGE_OFF:
                if (ts_active_req && safe) {
                    precharge_state = PRECHARGE_PRECHARGING;
                    precharge_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
                }
                break;
            case PRECHARGE_PRECHARGING:
                if (!safe) {
                    precharge_state = PRECHARGE_ERROR;
                } else if (hv_voltage > PRECHARGE_HV_THRESHOLD_V) {
                    precharge_state = PRECHARGE_ON;
                } else if (check_precharge_timeout(precharge_start)) {
                    precharge_state = PRECHARGE_ERROR;
                }
                break;
            case PRECHARGE_ON:
                if (!safe || !ts_active_req) {
                    precharge_state = PRECHARGE_OFF;
                    precharge_start = 0;
                }
                break;
            case PRECHARGE_ERROR:
                if (!ts_active_req) {
                    precharge_state = PRECHARGE_OFF;
                    precharge_start = 0;
                }
                break;
        }

        mosfet_driver_update();
        
        static uint32_t last_can_tx = 0;
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - last_can_tx > 100) {
            last_can_tx = xTaskGetTickCount() * portTICK_PERIOD_MS;
            can_service_send_precharge_state((uint8_t)precharge_state);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
