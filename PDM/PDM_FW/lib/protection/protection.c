#include "protection.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "fault_manager.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
static const char *TAG = "PDM_PROT";
#endif

// Nominal Currents (mA) for each of the 12 channels
static const float s_nominal_current[MUX_CHANNELS] = {
    2000.0f, 2000.0f, 2000.0f, 3000.0f, 2000.0f, 2000.0f,
    2000.0f, 2000.0f, 2000.0f, 2500.0f, 2000.0f, 2000.0f
};

static uint8_t  s_inv_sobre_consec = 0;    // Inverter channel (ch 9) inrush debounce
static uint8_t  s_volant_sobre_consec = 0; // Steering/Volant channel (ch 3) inrush debounce
static uint32_t s_tiempo_bajo_voltaje = 0;

// Multi-tier timer and warning tracking state per channel
static bool     s_timer_active[MUX_CHANNELS] = { false };
static uint32_t s_timer_start_ms[MUX_CHANNELS] = { 0 };
static bool     s_warning_active[MUX_CHANNELS] = { false };

void protection_init(void) {
    s_inv_sobre_consec = 0;
    s_volant_sobre_consec = 0;
    s_tiempo_bajo_voltaje = 0;
    for (int i = 0; i < MUX_CHANNELS; i++) {
        s_timer_active[i] = false;
        s_timer_start_ms[i] = 0;
        s_warning_active[i] = false;
    }
}

protection_level_t protection_check_channel(uint8_t ch, float current_ma, uint32_t current_time_ms) {
    if (ch >= MUX_CHANNELS) {
        return PROT_LEVEL_NORMAL;
    }

    // If channel is already off or locked, nothing to evaluate
    if (mosfet_driver_get_status(ch) == 0 || fault_manager_is_channel_locked(ch)) {
        s_timer_active[ch] = false;
        s_timer_start_ms[ch] = 0;
        s_warning_active[ch] = false;
        return PROT_LEVEL_NORMAL;
    }

    const float i_nom = s_nominal_current[ch];
    const float i_warn = i_nom * OVERCURRENT_WARN_RATIO;        // 110% (+10%)
    const float i_timer_low = i_nom * OVERCURRENT_TIMER_LOW_RATIO;  // 140%
    const float i_instant = i_nom * OVERCURRENT_INSTANT_RATIO;      // 170%

    // -------------------------------------------------------------
    // TIER 3: Instantaneous Trip (> 170% Inom)
    // -------------------------------------------------------------
    if (current_ma > i_instant) {
        bool trip_now = true;

        // Inrush debouncing (3 consecutive samples) for high inrush channels
        if (ch == CANAL_INVERTER) {
            s_inv_sobre_consec++;
            trip_now = (s_inv_sobre_consec >= 3);
        } else if (ch == CANAL_VOLANT) {
            s_volant_sobre_consec++;
            trip_now = (s_volant_sobre_consec >= 3);
        }

        if (trip_now) {
            fault_manager_lock_channel(ch);       // 1. Permanent lockout
            mosfet_driver_set_channel(ch, false); // 2. Instant physical cutoff
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, ch + 1); // 3. Diagnostic DTC

            s_timer_active[ch] = false;
            s_timer_start_ms[ch] = 0;
            s_warning_active[ch] = false;
            if (ch == CANAL_INVERTER) s_inv_sobre_consec = 0;
            if (ch == CANAL_VOLANT)   s_volant_sobre_consec = 0;

            return PROT_LEVEL_TRIPPED_INSTANT;
        }

        // Inrush sample pending
        s_warning_active[ch] = true;
        return PROT_LEVEL_WARNING_110;
    }

    // Reset inrush counters since current <= 170%
    if (ch == CANAL_INVERTER) s_inv_sobre_consec = 0;
    if (ch == CANAL_VOLANT)   s_volant_sobre_consec = 0;

    // -------------------------------------------------------------
    // TIER 2: Overload with 60s Timed Trip (140% <= I <= 170% Inom)
    // -------------------------------------------------------------
    if (current_ma >= i_timer_low) {
        s_warning_active[ch] = true;

        if (!s_timer_active[ch]) {
            // Start 60s countdown timer and send CAN alert / diagnostic notice
            s_timer_active[ch] = true;
            s_timer_start_ms[ch] = current_time_ms;
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_LOW, 70 + ch);
        }

        uint32_t elapsed_ms = current_time_ms - s_timer_start_ms[ch];
        if (elapsed_ms >= OVERCURRENT_TIMEOUT_MS) {
            // 60s expired without dropping <= 110% -> Cut off & Lockout
            fault_manager_lock_channel(ch);
            mosfet_driver_set_channel(ch, false);
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, ch + 1);

            s_timer_active[ch] = false;
            s_timer_start_ms[ch] = 0;
            s_warning_active[ch] = false;
            return PROT_LEVEL_TRIPPED_TIMED;
        }

        return PROT_LEVEL_TIMER_140_170;
    }

    // -------------------------------------------------------------
    // TIER 1: Warning / Hysteresis Range (110% < I < 140% Inom)
    // -------------------------------------------------------------
    if (current_ma > i_warn) {
        s_warning_active[ch] = true;

        if (s_timer_active[ch]) {
            // The 60s timer was previously started when current was in Tier 2.
            // Since current has not dropped <= 110% Inom, the timer continues!
            uint32_t elapsed_ms = current_time_ms - s_timer_start_ms[ch];
            if (elapsed_ms >= OVERCURRENT_TIMEOUT_MS) {
                fault_manager_lock_channel(ch);
                mosfet_driver_set_channel(ch, false);
                fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, ch + 1);

                s_timer_active[ch] = false;
                s_timer_start_ms[ch] = 0;
                s_warning_active[ch] = false;
                return PROT_LEVEL_TRIPPED_TIMED;
            }
            return PROT_LEVEL_TIMER_140_170;
        }

        // Timer not active: advisory warning message / CAN notification
        fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_LOW, 50 + ch);
        return PROT_LEVEL_WARNING_110;
    }

    // -------------------------------------------------------------
    // TIER 0: Normal Operation (I <= 110% Inom) -> Reset Timer & Warn
    // -------------------------------------------------------------
    s_warning_active[ch] = false;
    if (s_timer_active[ch]) {
        // Current dropped below 110% nominal -> Cancel / Reset timer
        s_timer_active[ch] = false;
        s_timer_start_ms[ch] = 0;
    }

    return PROT_LEVEL_NORMAL;
}

bool protection_check_channel_instant(uint8_t ch, float current_ma) {
    protection_level_t level = protection_check_channel(ch, current_ma, 0);
    return (level != PROT_LEVEL_TRIPPED_INSTANT && level != PROT_LEVEL_TRIPPED_TIMED);
}

bool protection_is_warning_active(uint8_t ch) {
    if (ch >= MUX_CHANNELS) return false;
    return s_warning_active[ch];
}

bool protection_is_timer_running(uint8_t ch) {
    if (ch >= MUX_CHANNELS) return false;
    return s_timer_active[ch];
}

uint32_t protection_get_timer_elapsed_ms(uint8_t ch, uint32_t current_time_ms) {
    if (ch >= MUX_CHANNELS || !s_timer_active[ch]) return 0;
    return (current_time_ms - s_timer_start_ms[ch]);
}

uint16_t protection_get_warning_mask(void) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < MUX_CHANNELS; i++) {
        if (s_warning_active[i]) {
            mask |= (uint16_t)(1U << i);
        }
    }
    return mask;
}

uint16_t protection_get_timer_active_mask(void) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < MUX_CHANNELS; i++) {
        if (s_timer_active[i]) {
            mask |= (uint16_t)(1U << i);
        }
    }
    return mask;
}

void protection_process_shunts_and_mux(uint16_t *out_consumos_can, uint32_t current_time_ms) {
    if (!out_consumos_can) return;

    uint32_t suma_lectures[MUX_CHANNELS] = { 0 };

    // 1. Loop of 10 measurements / averaging
    for (int volta = 0; volta < SAMPLES_PER_LOOP; volta++) {
        for (int ch = 0; ch < MUX_CHANNELS; ch++) {
            mux_adc_driver_select(ch);
            uint16_t lectura_actual = mux_adc_driver_read_raw();
            suma_lectures[ch] += lectura_actual;

            // Instantaneous current in mA
            float v_amp = (lectura_actual * (V_ESP / ADC_MAX));
            float corrent_instantanea = (v_amp * ESCALA_CORRIENTE * 1000.0f);

            // Multi-tier protection evaluation
            protection_level_t prot_res = protection_check_channel(ch, corrent_instantanea, current_time_ms);
            if (prot_res == PROT_LEVEL_TRIPPED_INSTANT || prot_res == PROT_LEVEL_TRIPPED_TIMED) {
                // Peak latching: freeze trip current in the CAN array
                out_consumos_can[ch] = (uint16_t)corrent_instantanea;
            }
        }
    }

    // 2. Average calculation for normal telemetry
    for (int ch = 0; ch < MUX_CHANNELS; ch++) {
        float v_amp_mitjana = (suma_lectures[ch] / (float)SAMPLES_PER_LOOP) * (V_ESP / ADC_MAX);
        float corriente_promedio = (v_amp_mitjana * ESCALA_CORRIENTE * 1000.0f);

        // Only overwrite if MOSFET is still ON
        if (mosfet_driver_get_status(ch) == 1) {
            out_consumos_can[ch] = (uint16_t)corriente_promedio;
        }
    }
}

void protection_process_hall_sensors(uint16_t *out_consumos_can) {
    if (!out_consumos_can) return;

    // Hall SD (Shutdown)
    uint16_t adc_sd = mux_adc_driver_read_pin(HALL_SD_PIN);
    float v_sd = (adc_sd * V_ESP) / ADC_MAX;
    float i_sd = (v_sd - V_OFF_HALL) / SENS_SD * 1000.0f;
    if (i_sd < 0) i_sd = 0;
    out_consumos_can[12] = (uint16_t)i_sd;

    // Hall Fans
    uint16_t adc_fans = mux_adc_driver_read_pin(HALL_FANS_PIN);
    float v_fans = (adc_fans * V_ESP) / ADC_MAX;
    float i_fans = (v_fans - V_OFF_HALL) / SENS_FANS * 1000.0f;
    if (i_fans < 0) i_fans = 0;
    out_consumos_can[13] = (uint16_t)i_fans;
}

bool protection_check_battery(float *out_vbat_actual, uint32_t current_time_ms) {
    uint16_t adc_vbat = mux_adc_driver_read_pin(V_SENSE_PIN);
    float v_pin = (adc_vbat * V_ESP) / ADC_MAX;
    float vbat = v_pin * (R2_DIV + R3_DIV) / R3_DIV;

    if (out_vbat_actual) {
        *out_vbat_actual = vbat;
    }

    if (vbat < VBAT_MIN_LIMIT_V) {
        if (s_tiempo_bajo_voltaje == 0) {
            s_tiempo_bajo_voltaje = current_time_ms;
        } else if (current_time_ms - s_tiempo_bajo_voltaje > VBAT_UNDERVOLTAGE_DEBOUNCE_MS) {
            for (int i = 0; i < MUX_CHANNELS; i++) {
                fault_manager_lock_channel(i);
            }
            mosfet_driver_set_all(false); // Cortar todo
            fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 99); // Error 99: Subtensión Batería
            return false; // Tripped
        }
    } else {
        s_tiempo_bajo_voltaje = 0;
    }

    return true; // OK
}

