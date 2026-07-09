#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_FAN_MOTOR   13
#define PIN_FAN_INV     14

#define ESC_MIN_US      1000
#define ESC_START_US    1140
#define ESC_MAX_US      2000

void fan_driver_init(void);
void fan_driver_set_speed(uint8_t fan_id, double pct);
double fan_driver_slew_pct(double current_pct, double target_pct, double dt_sec);
uint16_t fan_driver_pct_to_us(double pct);
uint32_t fan_driver_us_to_duty(uint16_t us);

#ifdef __cplusplus
}
#endif
