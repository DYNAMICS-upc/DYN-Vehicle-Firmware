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

void ipc_init(void);
bool ipc_send_mosfet_cmd(const mosfet_cmd_t* cmd);
bool ipc_receive_mosfet_cmd(mosfet_cmd_t* cmd);

#ifdef __cplusplus
}
#endif
