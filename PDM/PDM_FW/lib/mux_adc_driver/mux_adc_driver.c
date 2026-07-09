#include "mux_adc_driver.h"

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
#endif

static uint8_t s_s0 = 0;
static uint8_t s_s1 = 0;
static uint8_t s_s2 = 0;
static uint8_t s_s3 = 0;
static uint8_t s_sig_pin = 0;

void mux_adc_driver_init(uint8_t pin_s0, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t mux_sig_pin) {
    s_s0 = pin_s0;
    s_s1 = pin_s1;
    s_s2 = pin_s2;
    s_s3 = pin_s3;
    s_sig_pin = mux_sig_pin;
    
#if defined(ESP_PLATFORM)
    gpio_reset_pin((gpio_num_t)s_s0);
    gpio_set_direction((gpio_num_t)s_s0, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)s_s1);
    gpio_set_direction((gpio_num_t)s_s1, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)s_s2);
    gpio_set_direction((gpio_num_t)s_s2, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)s_s3);
    gpio_set_direction((gpio_num_t)s_s3, GPIO_MODE_OUTPUT);
    
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &s_adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(s_adc1_handle, (adc_channel_t)s_sig_pin, &config);
#endif
}

void mux_adc_driver_select(uint8_t channel) {
    uint8_t val_s0 = (channel & 0x01) ? 1 : 0;
    uint8_t val_s1 = (channel & 0x02) ? 1 : 0;
    uint8_t val_s2 = (channel & 0x04) ? 1 : 0;
    uint8_t val_s3 = (channel & 0x08) ? 1 : 0;
    
#if defined(ESP_PLATFORM)
    gpio_set_level((gpio_num_t)s_s0, val_s0);
    gpio_set_level((gpio_num_t)s_s1, val_s1);
    gpio_set_level((gpio_num_t)s_s2, val_s2);
    gpio_set_level((gpio_num_t)s_s3, val_s3);
    esp_rom_delay_us(20); // 20us settling time
#else
    (void)val_s0; (void)val_s1; (void)val_s2; (void)val_s3;
#endif
}

uint16_t mux_adc_driver_read_raw(void) {
#if defined(ESP_PLATFORM)
    int out = 0;
    if (s_adc1_handle) {
        adc_oneshot_read(s_adc1_handle, (adc_channel_t)s_sig_pin, &out);
    }
    return (uint16_t)out;
#else
    return 0;
#endif
}

uint16_t mux_adc_driver_read_pin(uint8_t pin) {
#if defined(ESP_PLATFORM)
    int out = 0;
    if (s_adc1_handle) {
        adc_oneshot_read(s_adc1_handle, (adc_channel_t)pin, &out);
    }
    return (uint16_t)out;
#else
    (void)pin;
    return 0;
#endif
}
