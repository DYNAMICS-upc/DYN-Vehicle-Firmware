#include "brake_driver.h"
#include <stddef.h>

#if defined(ESP_PLATFORM)
#include "esp_adc/adc_oneshot.h"
static adc_oneshot_unit_handle_t s_brake_adc = NULL;
#endif

void brake_driver_init(uint8_t pin) {
    (void)pin;
#if defined(ESP_PLATFORM)
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_cfg, &s_brake_adc);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(s_brake_adc, ADC_CHANNEL_7, &chan_cfg);
#endif
}

bool brake_driver_read(uint16_t* out_raw) {
    if (!out_raw) return false;

    int raw = 0;
#if defined(ESP_PLATFORM)
    if (s_brake_adc != NULL) {
        adc_oneshot_read(s_brake_adc, ADC_CHANNEL_7, &raw);
    }
#endif
    *out_raw = (uint16_t)raw;
    return true;
}

uint8_t brake_driver_get_percentage(uint16_t raw_val) {
    // Conversión 0-1100 a 0-100% según mcu.ino L668
    if (raw_val <= 0) return 0;
    if (raw_val >= 1100) return 100;
    return (uint8_t)((raw_val * 100) / 1100);
}
