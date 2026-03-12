#include "mcp_wrapper.h"

#ifdef ARDUINO
#include <mcp_can.h>

static MCP_CAN* s_mcp = nullptr;

void mcp_wrapper_init(uint8_t cs_pin) {
    if (s_mcp == nullptr) {
        s_mcp = new MCP_CAN(cs_pin);
    }
    s_mcp->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
    s_mcp->setMode(MCP_NORMAL);
}

bool mcp_wrapper_send_frame(uint32_t id, uint8_t* data, uint8_t len) {
    if (s_mcp == nullptr) return false;
    byte sndStat = s_mcp->sendMsgBuf(id, 0, len, data);
    return (sndStat == CAN_OK);
}

#else

void mcp_wrapper_init(uint8_t cs_pin) {
    (void)cs_pin;
}
bool mcp_wrapper_send_frame(uint32_t id, uint8_t* data, uint8_t len) {
    (void)id;
    (void)data;
    (void)len;
    return true;
}

#endif
