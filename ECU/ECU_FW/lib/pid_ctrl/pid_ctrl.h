#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PID_DIRECTION_DIRECT = 0,
    PID_DIRECTION_REVERSE = 1 // Para refrigeración (Temp > Setpoint -> Aumenta ventilación)
} pid_direction_t;

typedef struct {
    double kp;
    double ki;
    double kd;
    double setpoint;
    double integral;
    double prev_input;
    double out_min;
    double out_max;
    pid_direction_t direction;
} pid_ctrl_t;

void pid_ctrl_init(pid_ctrl_t* pid, double kp, double ki, double kd, double setpoint, pid_direction_t dir);
double pid_ctrl_compute(pid_ctrl_t* pid, double input, double dt_sec);
void pid_ctrl_reset(pid_ctrl_t* pid);

#ifdef __cplusplus
}
#endif
