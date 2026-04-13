#include "mux_adc_driver.h"

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
#else
extern void digitalWrite(uint8_t pin, int val);
extern int analogRead(uint8_t pin);
#endif

static uint8_t s_s0 = 0;
static uint8_t s_s1 = 0;
static uint8_t s_s2 = 0;
static uint8_t s_adc_pin = 0;

void mux_adc_driver_init(uint8_t pin_s0, uint8_t pin_s1, uint8_t pin_s2, uint8_t adc_pin) {
    s_s0 = pin_s0;
    s_s1 = pin_s1;
    s_s2 = pin_s2;
    s_adc_pin = adc_pin;
    
#if defined(ESP_PLATFORM)
    gpio_reset_pin((gpio_num_t)s_s0);
    gpio_set_direction((gpio_num_t)s_s0, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)s_s1);
    gpio_set_direction((gpio_num_t)s_s1, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)s_s2);
    gpio_set_direction((gpio_num_t)s_s2, GPIO_MODE_OUTPUT);
    
    // Basic init (assumes no conflict or ignores error)
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &s_adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_7, &config);
#endif
}

void mux_adc_driver_select(uint8_t channel) {
    uint8_t val_s0 = (channel & 0x01) ? 1 : 0;
    uint8_t val_s1 = (channel & 0x02) ? 1 : 0;
    uint8_t val_s2 = (channel & 0x04) ? 1 : 0;
    
#if defined(ESP_PLATFORM)
    gpio_set_level((gpio_num_t)s_s0, val_s0);
    gpio_set_level((gpio_num_t)s_s1, val_s1);
    gpio_set_level((gpio_num_t)s_s2, val_s2);
#else
    digitalWrite(s_s0, val_s0);
    digitalWrite(s_s1, val_s1);
    digitalWrite(s_s2, val_s2);
#endif
}

uint16_t mux_adc_driver_read(void) {
#if defined(ESP_PLATFORM)
    int out = 0;
    if (s_adc1_handle) {
        adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_7, &out);
    }
    return out;
#else
    return analogRead(s_adc_pin);
#endif
}
