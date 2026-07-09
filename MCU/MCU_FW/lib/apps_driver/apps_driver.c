#include "apps_driver.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined(ESP_PLATFORM)
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
#endif

static apps_data_t s_apps_data;

// Filtro de mediana de 3 muestras según mcu.ino L688-L693
static int16_t mediana3(int16_t a, int16_t b, int16_t c) {
    if (a > b) { int16_t t = a; a = b; b = t; }
    if (b > c) { int16_t t = b; b = c; c = t; }
    if (a > b) { int16_t t = a; a = b; b = t; }
    return b;
}

static inline int32_t map_val(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline float constrain_f(float val, float min_v, float max_v) {
    if (val < min_v) return min_v;
    if (val > max_v) return max_v;
    return val;
}

static inline int32_t constrain_i(int32_t val, int32_t min_v, int32_t max_v) {
    if (val < min_v) return min_v;
    if (val > max_v) return max_v;
    return val;
}

void apps_driver_init(uint8_t pin_main, uint8_t pin_sub) {
    (void)pin_main;
    (void)pin_sub;
    memset(&s_apps_data, 0, sizeof(s_apps_data));

#if defined(ESP_PLATFORM)
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config1, &s_adc1_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    // Configurar canales correspondientes a los pines EXT1 y EXT2
    adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_4, &chan_cfg);
    adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_5, &chan_cfg);
#endif
}

// Algoritmo calculateAPPS() de mcu.ino L695-L786
bool apps_driver_read(uint16_t* out_throttle) {
    static int16_t hist1[3] = {0, 0, 0};
    static int16_t hist2[3] = {0, 0, 0};
    static uint8_t hist_idx = 0;

    int raw_in1 = 0;
    int raw_in2 = 0;

#if defined(ESP_PLATFORM)
    if (s_adc1_handle != NULL) {
        adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_4, &raw_in1);
        adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_5, &raw_in2);
    }
#endif

    hist1[hist_idx] = (int16_t)raw_in1;
    hist2[hist_idx] = (int16_t)raw_in2;
    hist_idx = (hist_idx + 1) % 3;

    int16_t raw1 = mediana3(hist1[0], hist1[1], hist1[2]);
    int16_t raw2 = mediana3(hist2[0], hist2[1], hist2[2]);

    // Ventana = arco calibrado +- 15% (mcu.ino L709-L714)
    int lo1 = (APPS1_CAL_REPOSO < APPS1_CAL_FONDO) ? APPS1_CAL_REPOSO : APPS1_CAL_FONDO;
    int hi1 = (APPS1_CAL_REPOSO > APPS1_CAL_FONDO) ? APPS1_CAL_REPOSO : APPS1_CAL_FONDO;
    int lo2 = (APPS2_CAL_REPOSO < APPS2_CAL_FONDO) ? APPS2_CAL_REPOSO : APPS2_CAL_FONDO;
    int hi2 = (APPS2_CAL_REPOSO > APPS2_CAL_FONDO) ? APPS2_CAL_REPOSO : APPS2_CAL_FONDO;

    bool fallo1 = (raw1 < (int)(lo1 * 0.85f) || raw1 > (int)(hi1 * 1.15f));
    bool fallo2 = (raw2 < (int)(lo2 * 0.85f) || raw2 > (int)(hi2 * 1.15f));

    int16_t apps1 = (int16_t)constrain_i(map_val(raw1, APPS1_CAL_REPOSO, APPS1_CAL_FONDO, 0, 1000), 0, 1000);
    int16_t apps2 = (int16_t)constrain_i(map_val(raw2, APPS2_CAL_REPOSO, APPS2_CAL_FONDO, 0, 1000), 0, 1000);

    if (apps1 <= 0 || apps2 <= 0) {
        apps1 = 0;
        apps2 = 0;
    }

    s_apps_data.apps1_pct = (uint8_t)constrain_i(apps1 / 10, 0, 100);
    s_apps_data.apps2_pct = (uint8_t)constrain_i(apps2 / 10, 0, 100);

    static bool is_diff_over_threshold = false;
    static uint64_t diff_start_time_ms = 0;
    const uint64_t implausibility_time_ms = 100;

    float avg_pos = 0.0f;
    float diff_pct = (float)abs(apps1 - apps2) / 10.0f;
    bool implausible = fallo1 || fallo2 || (diff_pct >= 10.0f);

    s_apps_data.implausible_fault = implausible;
    s_apps_data.signal_cut = false;

    uint64_t current_time_ms = 0;
#if defined(ESP_PLATFORM)
    current_time_ms = (uint64_t)(esp_timer_get_time() / 1000ULL);
#endif

    if (implausible) {
        if (!is_diff_over_threshold) {
            diff_start_time_ms = current_time_ms;
            is_diff_over_threshold = true;
        }

        if ((current_time_ms - diff_start_time_ms) < implausibility_time_ms) {
            // Ventana de gracia de 100ms
            if (fallo1 && !fallo2) {
                avg_pos = (float)apps2;
            } else if (fallo2 && !fallo1) {
                avg_pos = (float)apps1;
            } else {
                avg_pos = (float)((apps1 < apps2) ? apps1 : apps2);
            }
        } else {
            avg_pos = 0.0f;
            s_apps_data.signal_cut = true;
        }
    } else {
        is_diff_over_threshold = false;
        avg_pos = (float)(apps1 + apps2) / 2.0f;
        if (avg_pos <= 10.0f) avg_pos = 0.0f;
        if (avg_pos > 990.0f) avg_pos = 1000.0f;
    }

    // Zona muerta configurable al inicio del recorrido (mcu.ino L778-L784)
    float deadband_counts = constrain_f(APPS_DEADBAND, 0.0f, 95.0f) * 10.0f;
    if (avg_pos <= deadband_counts) {
        avg_pos = 0.0f;
    } else {
        avg_pos = (avg_pos - deadband_counts) / (1000.0f - deadband_counts) * 1000.0f;
    }

    s_apps_data.throttle_cmd = constrain_f(avg_pos, 0.0f, 1000.0f);

    if (out_throttle) {
        *out_throttle = (uint16_t)s_apps_data.throttle_cmd;
    }

    return !s_apps_data.signal_cut;
}

void apps_driver_get_telemetry(apps_data_t* out_data) {
    if (out_data) {
        *out_data = s_apps_data;
    }
}
