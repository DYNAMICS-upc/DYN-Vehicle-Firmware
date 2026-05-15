#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t mosfet_id;
    bool enable;
} mosfet_cmd_t;

typedef struct {
    bool ts_active_req;
    uint16_t hv_voltage;
} vehicle_state_t;

void ipc_init(void);
bool ipc_send_mosfet_cmd(const mosfet_cmd_t* cmd);
bool ipc_receive_mosfet_cmd(mosfet_cmd_t* cmd);

bool ipc_send_vehicle_state(const vehicle_state_t* state);
bool ipc_peek_vehicle_state(vehicle_state_t* state);

#ifdef __cplusplus
}
#endif
