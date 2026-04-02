#include "apps_driver.h"
#include <stddef.h>

#if defined(ESP_PLATFORM)
#include "esp_adc/adc_oneshot.h"
adc_oneshot_unit_handle_t g_adc1_handle = NULL;
#endif

static uint8_t s_pin_main = 0;
static uint8_t s_pin_sub = 0;

#if !defined(ESP_PLATFORM)
static uint16_t s_mock_main = 0;
static uint16_t s_mock_sub = 0;
void apps_driver_set_mock(uint16_t main_val, uint16_t sub_val) {
    s_mock_main = main_val;
    s_mock_sub = sub_val;
}
int analogRead(uint8_t pin) {
    if (pin == s_pin_main) return s_mock_main;
    if (pin == s_pin_sub) return s_mock_sub;
    return 0;
}
#endif

void apps_driver_init(uint8_t pin_main, uint8_t pin_sub) {
    s_pin_main = pin_main;
    s_pin_sub = pin_sub;
#if defined(ESP_PLATFORM)
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &g_adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(g_adc1_handle, ADC_CHANNEL_6, &config);
    adc_oneshot_config_channel(g_adc1_handle, ADC_CHANNEL_7, &config);
#endif
}

bool apps_driver_read(uint16_t* out_val) {
    if (out_val == NULL) return false;
    
    int val_main = 0;
    int val_sub = 0;
#if defined(ESP_PLATFORM)
    int raw1 = 0, raw2 = 0;
    if (g_adc1_handle != NULL) {
        adc_oneshot_read(g_adc1_handle, ADC_CHANNEL_6, &raw1);
        adc_oneshot_read(g_adc1_handle, ADC_CHANNEL_7, &raw2);
    }
    val_main = raw1;
    val_sub = raw2;
#else
    val_main = analogRead(s_pin_main);
    val_sub = analogRead(s_pin_sub);
#endif
    
    // Plausibility check: main should be roughly double sub
    if (val_main > (val_sub * 2 + 100) || val_main < (val_sub * 2 - 100)) {
        return false;
    }
    
    *out_val = (uint16_t)val_main;
    return true;
}
