#include "fan_driver.h"
#include <math.h>
#include <string.h>

#define SLEW_RATE_MAX_PCT_PER_SEC 20.0

#if defined(ESP_PLATFORM)
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT
#define LEDC_FREQUENCY          50 // 50 Hz

#define LEDC_CH_FAN_MOTOR       LEDC_CHANNEL_0
#define LEDC_CH_FAN_INV         LEDC_CHANNEL_1
#endif

uint16_t fan_driver_pct_to_us(double pct) {
    if (pct <= 0.0) return ESC_MIN_US;
    if (pct >= 100.0) return ESC_MAX_US;
    return (uint16_t)(ESC_START_US + (pct / 100.0) * (ESC_MAX_US - ESC_START_US));
}

uint32_t fan_driver_us_to_duty(uint16_t us) {
    return (uint32_t)(((uint64_t)us * 16383ULL * 50ULL) / 1000000ULL);
}

double fan_driver_slew_pct(double current_pct, double target_pct, double dt_sec) {
    double max_step = SLEW_RATE_MAX_PCT_PER_SEC * dt_sec;
    double delta = target_pct - current_pct;
    if (fabs(delta) <= max_step) return target_pct;
    return current_pct + (delta > 0.0 ? max_step : -max_step);
}

void fan_driver_init(void) {
#if defined(ESP_PLATFORM)
    ledc_timer_config_t ledc_timer;
    memset(&ledc_timer, 0, sizeof(ledc_timer));
    ledc_timer.speed_mode       = LEDC_MODE;
    ledc_timer.duty_resolution  = LEDC_DUTY_RES;
    ledc_timer.timer_num        = LEDC_TIMER;
    ledc_timer.freq_hz          = LEDC_FREQUENCY;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ch_motor;
    memset(&ch_motor, 0, sizeof(ch_motor));
    ch_motor.gpio_num       = PIN_FAN_MOTOR;
    ch_motor.speed_mode     = LEDC_MODE;
    ch_motor.channel        = LEDC_CH_FAN_MOTOR;
    ch_motor.timer_sel      = LEDC_TIMER;
    ch_motor.duty           = fan_driver_us_to_duty(ESC_MIN_US);
    ch_motor.hpoint         = 0;
    ledc_channel_config(&ch_motor);

    ledc_channel_config_t ch_inv;
    memset(&ch_inv, 0, sizeof(ch_inv));
    ch_inv.gpio_num       = PIN_FAN_INV;
    ch_inv.speed_mode     = LEDC_MODE;
    ch_inv.channel        = LEDC_CH_FAN_INV;
    ch_inv.timer_sel      = LEDC_TIMER;
    ch_inv.duty           = fan_driver_us_to_duty(ESC_MIN_US);
    ch_inv.hpoint         = 0;
    ledc_channel_config(&ch_inv);
#endif
}

void fan_driver_set_speed(uint8_t fan_id, double pct) {
    if (pct < 0.0) pct = 0.0;
    if (pct > 100.0) pct = 100.0;

#if defined(ESP_PLATFORM)
    uint16_t us = fan_driver_pct_to_us(pct);
    uint32_t duty = fan_driver_us_to_duty(us);
    ledc_channel_t ch = (fan_id == 0) ? LEDC_CH_FAN_MOTOR : LEDC_CH_FAN_INV;
    ledc_set_duty(LEDC_MODE, ch, duty);
    ledc_update_duty(LEDC_MODE, ch);
#else
    (void)fan_id;
#endif
}
