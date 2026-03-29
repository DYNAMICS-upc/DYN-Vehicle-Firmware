#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void nextion_driver_init(void);
void nextion_driver_init_dma(void);
void nextion_driver_send_cmd(const char* cmd);

#ifdef __cplusplus
}
#endif
