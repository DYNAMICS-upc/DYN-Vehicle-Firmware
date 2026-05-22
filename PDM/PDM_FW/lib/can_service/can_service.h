#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void can_service_init(void);
void can_service_send_precharge_state(uint8_t state);

#ifdef __cplusplus
}
#endif
