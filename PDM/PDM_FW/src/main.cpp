#include <stdio.h>
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

extern "C" void app_main(void) {
    mosfet_driver_init(MOSFET_PIN_LATCH, MOSFET_PIN_ENABLE);
    mux_adc_driver_init(MUX_PIN_S0, MUX_PIN_S1, MUX_PIN_S2, MUX_PIN_SIG);
    protection_init();
    ipc_init();
    can_service_init();

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
        
        bool ts_active_req = false; // Mock TS request
        uint16_t hv_voltage = 0; // Mock HV voltage
        
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
                } else if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - precharge_start > PRECHARGE_TIMEOUT_MS) {
                    precharge_state = PRECHARGE_ERROR;
                }
                break;
            case PRECHARGE_ON:
                if (!safe || !ts_active_req) {
                    precharge_state = PRECHARGE_OFF;
                }
                break;
            case PRECHARGE_ERROR:
                if (!ts_active_req) {
                    precharge_state = PRECHARGE_OFF;
                }
                break;
        }

        mosfet_driver_update();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
