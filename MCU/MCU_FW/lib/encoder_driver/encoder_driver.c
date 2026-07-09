#include "encoder_driver.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// Variables volátiles para interrupciones (Medición de período en microsegundos según mcu.ino L208-L222)
static volatile uint64_t s_last_micros_fl = 0;
static volatile uint64_t s_period_fl = 0;
static volatile uint32_t s_pulse_count_fl = 0;

static volatile uint64_t s_last_micros_fr = 0;
static volatile uint64_t s_period_fr = 0;
static volatile uint32_t s_pulse_count_fr = 0;

static volatile uint64_t s_last_micros_rl = 0;
static volatile uint64_t s_period_rl = 0;
static volatile uint32_t s_pulse_count_rl = 0;

static volatile uint64_t s_last_micros_rr = 0;
static volatile uint64_t s_period_rr = 0;
static volatile uint32_t s_pulse_count_rr = 0;

// ISRs exactas de mcu.ino L244-L270
static void IRAM_ATTR isr_fl(void *arg) {
    (void)arg;
    uint64_t current_micros = (uint64_t)esp_timer_get_time();
    s_period_fl = current_micros - s_last_micros_fl;
    s_last_micros_fl = current_micros;
    s_pulse_count_fl++;
}

static void IRAM_ATTR isr_fr(void *arg) {
    (void)arg;
    uint64_t current_micros = (uint64_t)esp_timer_get_time();
    s_period_fr = current_micros - s_last_micros_fr;
    s_last_micros_fr = current_micros;
    s_pulse_count_fr++;
}

static void IRAM_ATTR isr_rl(void *arg) {
    (void)arg;
    uint64_t current_micros = (uint64_t)esp_timer_get_time();
    s_period_rl = current_micros - s_last_micros_rl;
    s_last_micros_rl = current_micros;
    s_pulse_count_rl++;
}

static void IRAM_ATTR isr_rr(void *arg) {
    (void)arg;
    uint64_t current_micros = (uint64_t)esp_timer_get_time();
    s_period_rr = current_micros - s_last_micros_rr;
    s_last_micros_rr = current_micros;
    s_pulse_count_rr++;
}

static wheel_speeds_t s_speeds;
static portMUX_TYPE s_enc_mux = portMUX_INITIALIZER_UNLOCKED;

void encoder_driver_init(void) {
    memset(&s_speeds, 0, sizeof(s_speeds));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_ENC_FL) | (1ULL << PIN_ENC_FR) | 
                        (1ULL << PIN_ENC_RL) | (1ULL << PIN_ENC_RR),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // FALLING según mcu.ino L355-L358
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)PIN_ENC_FL, isr_fl, NULL);
    gpio_isr_handler_add((gpio_num_t)PIN_ENC_FR, isr_fr, NULL);
    gpio_isr_handler_add((gpio_num_t)PIN_ENC_RL, isr_rl, NULL);
    gpio_isr_handler_add((gpio_num_t)PIN_ENC_RR, isr_rr, NULL);
}

// Algoritmo exacto calculateWheelSpeeds() de mcu.ino L592-L660
void encoder_driver_update(void) {
    uint64_t current_micros = (uint64_t)esp_timer_get_time();

    uint64_t safe_period_fl, safe_last_micros_fl;
    uint64_t safe_period_fr, safe_last_micros_fr;
    uint64_t safe_period_rl, safe_last_micros_rl;
    uint64_t safe_period_rr, safe_last_micros_rr;

    portENTER_CRITICAL(&s_enc_mux);
    safe_period_fl = s_period_fl;
    safe_last_micros_fl = s_last_micros_fl;
    safe_period_fr = s_period_fr;
    safe_last_micros_fr = s_last_micros_fr;
    safe_period_rl = s_period_rl;
    safe_last_micros_rl = s_last_micros_rl;
    safe_period_rr = s_period_rr;
    safe_last_micros_rr = s_last_micros_rr;
    portEXIT_CRITICAL(&s_enc_mux);

    float speed_ms_fl = 0.0f;
    float speed_ms_fr = 0.0f;
    float speed_ms_rl = 0.0f;
    float speed_ms_rr = 0.0f;
    s_speeds.rpm_fl = 0.0f;
    s_speeds.rpm_fr = 0.0f;
    s_speeds.rpm_rl = 0.0f;
    s_speeds.rpm_rr = 0.0f;

    // 150000us -> ~1 km/h de velocidad mínima detectable (mcu.ino L621)
    const uint64_t TIMEOUT_MICROS = 150000;

    // Front Left (600 PPR)
    if ((current_micros - safe_last_micros_fl) < TIMEOUT_MICROS && safe_period_fl > 0) {
        float hz_fl = 1000000.0f / (float)safe_period_fl;
        float rps_fl = hz_fl / ENCODER_FRONT_PPR;
        s_speeds.rpm_fl = rps_fl * 60.0f;
        speed_ms_fl = rps_fl * 2.0f * 3.14159265f * WHEEL_RADIUS;
    }

    // Front Right (600 PPR)
    if ((current_micros - safe_last_micros_fr) < TIMEOUT_MICROS && safe_period_fr > 0) {
        float hz_fr = 1000000.0f / (float)safe_period_fr;
        float rps_fr = hz_fr / ENCODER_FRONT_PPR;
        s_speeds.rpm_fr = rps_fr * 60.0f;
        speed_ms_fr = rps_fr * 2.0f * 3.14159265f * WHEEL_RADIUS;
    }

    // Rear Left (60 PPR)
    if ((current_micros - safe_last_micros_rl) < TIMEOUT_MICROS && safe_period_rl > 0) {
        float hz_rl = 1000000.0f / (float)safe_period_rl;
        float rps_rl = hz_rl / ENCODER_REAR_PPR;
        s_speeds.rpm_rl = rps_rl * 60.0f;
        speed_ms_rl = rps_rl * 2.0f * 3.14159265f * WHEEL_RADIUS;
    }

    // Rear Right (60 PPR)
    if ((current_micros - safe_last_micros_rr) < TIMEOUT_MICROS && safe_period_rr > 0) {
        float hz_rr = 1000000.0f / (float)safe_period_rr;
        float rps_rr = hz_rr / ENCODER_REAR_PPR;
        s_speeds.rpm_rr = rps_rr * 60.0f;
        speed_ms_rr = rps_rr * 2.0f * 3.14159265f * WHEEL_RADIUS;
    }

    // Guardar para telemetría CAN (m/s * 100) según mcu.ino L656-L659
    s_speeds.speed_fl_cms = (uint16_t)(speed_ms_fl * 100.0f);
    s_speeds.speed_fr_cms = (uint16_t)(speed_ms_fr * 100.0f);
    s_speeds.speed_rl_cms = (uint16_t)(speed_ms_rl * 100.0f);
    s_speeds.speed_rr_cms = (uint16_t)(speed_ms_rr * 100.0f);

    s_speeds.speed_front_avg = (uint32_t)((speed_ms_fl + speed_ms_fr) * 50.0f);
    s_speeds.speed_rear_avg  = (uint32_t)((speed_ms_rl + speed_ms_rr) * 50.0f);
}

void encoder_driver_get_speeds(wheel_speeds_t* out) {
    if (out) {
        *out = s_speeds;
    }
}

#else

static wheel_speeds_t s_speeds;

void encoder_driver_init(void) {
    memset(&s_speeds, 0, sizeof(s_speeds));
}

void encoder_driver_update(void) {}

void encoder_driver_get_speeds(wheel_speeds_t* out) {
    if (out) {
        *out = s_speeds;
    }
}

#endif
