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

static uint8_t s_inv_sobre_consec = 0;   // Inverter channel (ch 9)
static uint8_t s_volant_sobre_consec = 0;// Steering/Volant channel (ch 3)
static uint32_t s_tiempo_bajo_voltaje = 0;

void protection_init(void) {
    s_inv_sobre_consec = 0;
    s_volant_sobre_consec = 0;
    s_tiempo_bajo_voltaje = 0;
}

bool protection_check_channel_instant(uint8_t ch, float corrent_instantanea) {
    if (ch >= MUX_CHANNELS) return true;

    if (mosfet_driver_get_status(ch) == 1) {
        if (corrent_instantanea > s_nominal_current[ch] * 1.3f) {
            bool tallar = true;

            // Inverter (ch 9) and Volant (ch 3) require 3 consecutive samples
            if (ch == CANAL_INVERTER) {
                s_inv_sobre_consec++;
                tallar = (s_inv_sobre_consec >= 3);
            } else if (ch == CANAL_VOLANT) {
                s_volant_sobre_consec++;
                tallar = (s_volant_sobre_consec >= 3);
            }

            if (tallar) {
                fault_manager_lock_channel(ch);       // 1. Bloqueo permanente de reactivación
                mosfet_driver_set_channel(ch, false); // 2. Corte físico del MOSFET
                fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, ch + 1); // 3. Registro de causa
                
                if (ch == CANAL_INVERTER) s_inv_sobre_consec = 0;
                if (ch == CANAL_VOLANT) s_volant_sobre_consec = 0;
                
                return false; // Tripped
            }
        } else {
            if (ch == CANAL_INVERTER) s_inv_sobre_consec = 0;
            if (ch == CANAL_VOLANT) s_volant_sobre_consec = 0;
        }
    }
    return true; // OK
}

void protection_process_shunts_and_mux(uint16_t *out_consumos_can) {
    if (!out_consumos_can) return;

    uint32_t suma_lectures[MUX_CHANNELS] = { 0 };

    // 1. Bucle de 10 medidas/voltes
    for (int volta = 0; volta < SAMPLES_PER_LOOP; volta++) {
        for (int ch = 0; ch < MUX_CHANNELS; ch++) {
            mux_adc_driver_select(ch);
            uint16_t lectura_actual = mux_adc_driver_read_raw();
            suma_lectures[ch] += lectura_actual;

            // Corriente instantanea en mA
            float v_amp = (lectura_actual * (V_ESP / ADC_MAX));
            float corrent_instantanea = (v_amp * ESCALA_CORRIENTE * 1000.0f);

            // Evaluacion de proteccion activa
            if (!protection_check_channel_instant(ch, corrent_instantanea)) {
                // Peak latching: bloquear valor de pico en el array CAN
                out_consumos_can[ch] = (uint16_t)corrent_instantanea;
            }
        }
    }

    // 2. Calculo del promedio de las 10 muestras para telemetria CAN normal
    for (int ch = 0; ch < MUX_CHANNELS; ch++) {
        float v_amp_mitjana = (suma_lectures[ch] / (float)SAMPLES_PER_LOOP) * (V_ESP / ADC_MAX);
        float corriente_promedio = (v_amp_mitjana * ESCALA_CORRIENTE * 1000.0f);

        // Solo sobreescribir si el MOSFET sigue encendido (evitar borrar picos)
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
