#include "can_driver.h"
#include "mcp_wrapper.h"
#include <stddef.h>

static uint8_t s_cs_pin = 0;
static uint32_t s_last_sent_id = 0;

void can_driver_init(uint8_t cs_pin) {
    s_cs_pin = cs_pin;
    mcp_wrapper_init(s_cs_pin);
}

bool can_driver_send_frame(uint32_t id, uint8_t* data, uint8_t len) {
    if (data == NULL || len == 0 || len > 8) {
        return false;
    }

    bool success = mcp_wrapper_send_frame(id, data, len);
    if (success) {
        s_last_sent_id = id;
    }
    return success;
}

uint32_t can_driver_get_last_id(void) {
    return s_last_sent_id;
}
