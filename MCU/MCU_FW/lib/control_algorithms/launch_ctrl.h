#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void launch_ctrl_init(void);
int32_t launch_ctrl_update(uint32_t speed_front, uint32_t speed_rear);

#ifdef __cplusplus
}
#endif
