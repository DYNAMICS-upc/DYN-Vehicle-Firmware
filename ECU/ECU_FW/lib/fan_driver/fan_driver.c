#include "fan_driver.h"

// Hardware dependencies mapped for ESP32 and native
#if defined(ESP_PLATFORM)
#include "driver/ledc.h"
#endif

static uint8_t s_pin = 0;

#if !defined(ESP_PLATFORM)
// Mocks for native testing
static uint8_t s_mock_speed = 0;
uint8_t fan_driver_get_mock_speed(void) {
    return s_mock_speed;
}
#endif

void fan_driver_init(uint8_t pin) {
    s_pin = pin;
#if defined(ESP_PLATFORM)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT, // 0-255
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 25000, // 25 kHz is standard for PC fans
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = s_pin,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
#endif
}

void fan_driver_set_speed(uint8_t duty_cycle) {
#if defined(ESP_PLATFORM)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
#else
    s_mock_speed = duty_cycle;
#endif
}
