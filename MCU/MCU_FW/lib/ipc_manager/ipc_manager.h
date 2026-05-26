#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t length;
} mcu_can_msg_t;

void ipc_manager_init(void);
bool ipc_manager_send_can_msg(const mcu_can_msg_t* msg);
bool ipc_manager_receive_can_msg(mcu_can_msg_t* msg);

#ifdef __cplusplus
}
#endif
