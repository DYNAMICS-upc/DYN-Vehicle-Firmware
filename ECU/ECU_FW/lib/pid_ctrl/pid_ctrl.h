#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t kp;
    int32_t ki;
    int32_t kd;
    int32_t target;
    int32_t error_integral;
    int32_t prev_error;
} pid_ctrl_t;

void pid_ctrl_init(pid_ctrl_t* pid, int32_t kp, int32_t ki, int32_t kd, int32_t target);
uint8_t pid_ctrl_compute(pid_ctrl_t* pid, int32_t current_val);

#ifdef __cplusplus
}
#endif
