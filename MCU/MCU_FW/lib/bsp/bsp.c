#include "bsp.h"

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t s_bsp_adc_handle = NULL;

void bsp_init(void) {
    gpio_config_t in_conf = {
        .pin_bit_mask = (1ULL << PIN_SDCB) | (1ULL << PIN_DASHB),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&in_conf);

    gpio_config_t out_conf = {
        .pin_bit_mask = (1ULL << PIN_R2D_BUZZER) | (1ULL << PIN_BRAKE_LIGHT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_conf);

    gpio_set_level((gpio_num_t)PIN_R2D_BUZZER, 0);
    gpio_set_level((gpio_num_t)PIN_BRAKE_LIGHT, 0);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config, &s_bsp_adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(s_bsp_adc_handle, ADC_CHANNEL_6, &chan_cfg); // GPIO 17 (STEERING)
}

bool bsp_read_ts_active(void) {
    // SDCb activo en nivel bajo según circuitería FS
    return gpio_get_level((gpio_num_t)PIN_SDCB) == 0;
}

bool bsp_read_dash_button(void) {
    return gpio_get_level((gpio_num_t)PIN_DASHB) == 0;
}

void bsp_set_r2d_buzzer(bool active) {
    gpio_set_level((gpio_num_t)PIN_R2D_BUZZER, active ? 1 : 0);
}

void bsp_set_brake_light(bool active) {
    gpio_set_level((gpio_num_t)PIN_BRAKE_LIGHT, active ? 1 : 0);
}

int16_t bsp_read_steering_angle(void) {
    int raw = 0;
    if (s_bsp_adc_handle != NULL) {
        adc_oneshot_read(s_bsp_adc_handle, ADC_CHANNEL_6, &raw);
    }
    // Mapeo 0..4095 a -180..+180 grados
    float deg = ((float)raw / 4095.0f * 360.0f) - 180.0f;
    return (int16_t)deg;
}

#else

void bsp_init(void) {}
bool bsp_read_ts_active(void) { return true; }
bool bsp_read_dash_button(void) { return false; }
void bsp_set_r2d_buzzer(bool active) { (void)active; }
void bsp_set_brake_light(bool active) { (void)active; }
int16_t bsp_read_steering_angle(void) { return 0; }

#endif
