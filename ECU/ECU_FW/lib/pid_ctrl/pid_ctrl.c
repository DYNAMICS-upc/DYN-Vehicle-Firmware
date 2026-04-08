#include "pid_ctrl.h"

void pid_ctrl_init(pid_ctrl_t* pid, int32_t kp, int32_t ki, int32_t kd, int32_t target) {
    if (!pid) return;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->target = target;
    pid->error_integral = 0;
    pid->prev_error = 0;
}

uint8_t pid_ctrl_compute(pid_ctrl_t* pid, int32_t current_val) {
    if (!pid) return 0;
    
    int32_t error = pid->target - current_val;
    pid->error_integral += error;
    int32_t error_deriv = error - pid->prev_error;
    pid->prev_error = error;
    
    int32_t output = (pid->kp * error) + (pid->ki * pid->error_integral) + (pid->kd * error_deriv);
    
    // Anti-windup & saturation
    if (output > 255) output = 255;
    if (output < 0) output = 0;
    
    return (uint8_t)output;
}
