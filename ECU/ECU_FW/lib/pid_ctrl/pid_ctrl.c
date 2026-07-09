#include "pid_ctrl.h"

void pid_ctrl_init(pid_ctrl_t* pid, double kp, double ki, double kd, double setpoint, pid_direction_t dir) {
    if (!pid) return;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->integral = 0.0;
    pid->prev_input = setpoint;
    pid->out_min = 0.0;
    pid->out_max = 100.0;
    pid->direction = dir;
}

double pid_ctrl_compute(pid_ctrl_t* pid, double input, double dt_sec) {
    if (!pid || dt_sec <= 0.0) return 0.0;

    double error = (pid->direction == PID_DIRECTION_REVERSE) 
                   ? (input - pid->setpoint) 
                   : (pid->setpoint - input);

    // Integración con Anti-Windup
    pid->integral += error * dt_sec;
    if (pid->integral > pid->out_max) pid->integral = pid->out_max;
    if (pid->integral < pid->out_min) pid->integral = pid->out_min;

    // Derivada sobre la entrada para evitar derivative kick
    double d_input = (input - pid->prev_input) / dt_sec;
    pid->prev_input = input;

    double output = (pid->kp * error) + (pid->ki * pid->integral) - (pid->kd * d_input);

    if (output > pid->out_max) output = pid->out_max;
    if (output < pid->out_min) output = pid->out_min;

    return output;
}

void pid_ctrl_reset(pid_ctrl_t* pid) {
    if (pid) {
        pid->integral = 0.0;
        pid->prev_input = pid->setpoint;
    }
}
