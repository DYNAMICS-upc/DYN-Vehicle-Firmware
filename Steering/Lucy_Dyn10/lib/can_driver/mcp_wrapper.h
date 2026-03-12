#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mcp_wrapper_init(uint8_t cs_pin);
bool mcp_wrapper_send_frame(uint32_t id, uint8_t* data, uint8_t len);

#ifdef __cplusplus
}
#endif
