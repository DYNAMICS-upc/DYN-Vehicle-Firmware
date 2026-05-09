#include "can_service.h"
#include "can_driver.h"
#include "ipc.h"
#include "volante_state.h"
#include <Arduino_FreeRTOS.h>
#include <task.h>

void can_service_init(void) {
    // any initialization if needed
}

void can_service_update(void) {
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
    
    if (can_driver_receive_frame(&id, data, &len)) {
        volante_state_t state;
        if (ipc_receive_state(&state)) {
            // Structured parsing
            if (id == 0x100) { // BMS status
                state.has_bms_fault = (data[0] != 0);
                state.dash.bms_fault = state.has_bms_fault;
                state.dash.soc_percent = data[1];
                state.dash.bat_max_temp = data[2];
                state.dash.hv_voltage = (data[3] << 8) | data[4];
            } else if (id == 0x101) { // IMD status
                state.has_imd_fault = (data[0] != 0);
                state.dash.imd_fault = state.has_imd_fault;
            } else if (id == 0x200) { // Inverter data
                state.dash.speed_kmh = data[0];
                state.dash.motor_temp = data[1];
                state.dash.inv_temp = data[2];
                state.dash.is_r2d = (data[3] != 0);
                state.dash.inv_fault = (data[4] != 0);
            }
            ipc_send_state(&state);
        }
    }
    
    // Envio periódico del estado del volante via CAN
    static TickType_t last_send = 0;
    TickType_t now = xTaskGetTickCount();
    if (now - last_send >= pdMS_TO_TICKS(100)) {
        last_send = now;
        volante_state_t state;
        if (ipc_peek_state(&state)) {
            uint8_t tx_data[8] = {0};
            tx_data[0] = state.btn_launch_pressed ? 1 : 0;
            can_driver_send_frame(0x301, tx_data, 1);
        }
    }
}
