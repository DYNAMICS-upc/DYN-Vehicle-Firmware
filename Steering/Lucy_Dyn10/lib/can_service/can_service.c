#include "can_service.h"
#include "can_driver.h"
#include "ipc.h"
#include "volante_state.h"

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
            } else if (id == 0x101) { // IMD status
                state.has_imd_fault = (data[0] != 0);
            }
            ipc_send_state(&state);
        }
    }
}
