#include "app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "protection.h"
#include "can_service.h"
#include "pdm_config.h"
#include "fault_manager.h"
#include "ota_service.h"

static uint16_t s_consumos_can[TOTAL_LOADS] = { 0 };
static float s_v_bat_actual = 12.0f;

void app_init(void) {
    mosfet_driver_init();
    mux_adc_driver_init(MUX_PIN_S0, MUX_PIN_S1, MUX_PIN_S2, MUX_PIN_S3, MUX_COMMON_PIN);
    protection_init();
    can_service_init();
    fault_manager_init();
}

void app_run(void) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t last_can_tx_ms = 0;

    while (1) {
        uint32_t current_time_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        // 1. Verificación de subtensión de batería con debounce de 200ms
        protection_check_battery(&s_v_bat_actual, current_time_ms);

        // 2. Procesamiento de los 12 canales MUX con promedio de 10 muestras y fusible rápido
        protection_process_shunts_and_mux(s_consumos_can);

        // 3. Procesamiento de sensores Hall (Shutdown y Ventiladores)
        protection_process_hall_sensors(s_consumos_can);

        // 4. Verificación de alertas de bus CAN (Bus-Off / Errores de trama)
        can_service_check_alerts();

        // 5. Envío periódico de la tabla CAN completa a 10 Hz (100 ms)
        if (current_time_ms - last_can_tx_ms >= CAN_INTERVAL_MS) {
            last_can_tx_ms = current_time_ms;
            uint8_t mosfets_status[MUX_CHANNELS];
            mosfet_driver_get_all_statuses(mosfets_status);
            can_service_send_all_telemetry(mosfets_status, s_consumos_can, s_v_bat_actual);
        }

        // Lazo determinista de 10 ms (100 Hz)
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}
