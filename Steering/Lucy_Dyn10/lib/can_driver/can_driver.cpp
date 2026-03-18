#include "can_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <mcp_can.h>

static MCP_CAN* s_mcp = nullptr;
#endif

static uint8_t s_cs_pin = 0;
static uint32_t s_last_sent_id = 0;

void can_driver_init(uint8_t cs_pin) {
    s_cs_pin = cs_pin;
#ifdef ARDUINO
    if (s_mcp == nullptr) {
        s_mcp = new MCP_CAN(s_cs_pin);
    }
    s_mcp->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
    s_mcp->setMode(MCP_NORMAL);
#endif
}

bool can_driver_send_frame(uint32_t id, uint8_t* data, uint8_t len) {
    if (data == NULL || len == 0 || len > 8) {
        return false;
    }

#ifdef ARDUINO
    if (s_mcp == nullptr) return false;
    byte sndStat = s_mcp->sendMsgBuf(id, 0, len, data);
    if (sndStat != CAN_OK) {
        return false;
    }
#endif

    s_last_sent_id = id;
    return true;
}

uint32_t can_driver_get_last_id(void) {
    return s_last_sent_id;
}
