#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void torque_ctrl_init(void);
int32_t torque_ctrl_calculate(uint32_t throttle_raw, uint32_t speed_rpm, bool brake_pressed, bool r2d_active, int32_t slip_multiplier);

#ifdef __cplusplus
}
#endif
