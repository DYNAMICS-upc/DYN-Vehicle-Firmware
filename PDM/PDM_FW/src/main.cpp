#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "protection.h"
#include "ipc.h"
#include "can_service.h"

extern "C" void app_main(void) {
    mosfet_driver_init(18, 34); // Example ESP32 pins
    mux_adc_driver_init(25, 26, 27, 32); // Example S0, S1, S2, SIG pins
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
        for (uint8_t i = 0; i < 8; i++) {
            mux_adc_driver_select(i);
            uint16_t adc_val = mux_adc_driver_read();
            if (!protection_check_mux_channel(i, adc_val)) {
                safe = false;
            }
        }
        
        uint16_t vbat_val = 12500; // Mock 12.5V (en mV)
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

        mosfet_driver_update();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
