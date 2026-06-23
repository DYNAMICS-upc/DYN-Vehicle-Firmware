#include "mosfet_driver.h"
#include <stddef.h>

#if defined(ESP_PLATFORM)
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
static adc_oneshot_unit_handle_t s_adc1_handle;
#endif
extern void digitalWrite(uint8_t pin, int val);
extern int analogRead(uint8_t pin);
#define LOW 0

static uint8_t s_ctrl_pin = 0;
static uint8_t s_sense_pin = 0;
static uint8_t s_fault_counter = 0;
#define FAULT_FILTER_LIMIT 5
static const uint16_t MAX_CURRENT_THRESHOLD = 800; // Arbitrary ADC limit for mock

#if !defined(ESP_PLATFORM)
static uint16_t s_mock_current = 0;
void mosfet_driver_set_mock_current(uint16_t current_val) {
    s_mock_current = current_val;
}
int analogRead(uint8_t pin) {
    if (pin == s_sense_pin) return s_mock_current;
    return 0;
}
void digitalWrite(uint8_t pin, int val) {
    (void)pin;
    (void)val;
}
#endif

void mosfet_driver_init(uint8_t ctrl_pin, uint8_t sense_pin) {
    s_ctrl_pin = ctrl_pin;
    s_sense_pin = sense_pin;
    s_fault_counter = 0; // REINICIO ESTRICTO PARA TESTS
#if defined(ESP_PLATFORM)
    gpio_reset_pin((gpio_num_t)s_ctrl_pin);
    gpio_set_direction((gpio_num_t)s_ctrl_pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)s_ctrl_pin, 0); // Asegurar apagado inicial
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &s_adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_6, &config);
#else
    digitalWrite(s_ctrl_pin, LOW); // Asegurar apagado nativo
#endif
}

void mosfet_driver_set(bool state) {
#if defined(ESP_PLATFORM)
    gpio_set_level((gpio_num_t)s_ctrl_pin, state ? 1 : 0);
#else
    digitalWrite(s_ctrl_pin, state ? 1 : 0);
#endif
}

bool mosfet_driver_check_fault(void) {
#if defined(ESP_PLATFORM)
    int current = 0;
    adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_6, &current);
#else
    int current = analogRead(s_sense_pin);
#endif

    if (current > MAX_CURRENT_THRESHOLD) {
        s_fault_counter++;
        if (s_fault_counter >= FAULT_FILTER_LIMIT) {
#if defined(ESP_PLATFORM)
            gpio_set_level((gpio_num_t)s_ctrl_pin, 0); // Force off on fault
#else
            digitalWrite(s_ctrl_pin, 0); // Force off on fault
#endif
            return true; // Fault detected
        }
        return false; // Soft-start tolerance
    }
    
    s_fault_counter = 0;
    return false;
}

void mosfet_driver_update(void) {
    if (mosfet_driver_check_fault()) {
        // Logica interna de disparo: apagar mosfets y notificar si es necesario
        mosfet_driver_set(false);
    }
}
