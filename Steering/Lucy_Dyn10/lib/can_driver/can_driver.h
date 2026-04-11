#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void can_driver_init(uint8_t cs_pin);
bool can_driver_send_frame(uint32_t id, uint8_t* data, uint8_t len);
bool can_driver_receive_frame(uint32_t* id, uint8_t* data, uint8_t* len);

// For testing purposes
uint32_t can_driver_get_last_id(void);

#ifdef __cplusplus
}
#endif
