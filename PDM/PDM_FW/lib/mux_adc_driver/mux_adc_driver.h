#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mux_adc_driver_init(uint8_t pin_s0, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t mux_sig_pin);
void mux_adc_driver_select(uint8_t channel);
uint16_t mux_adc_driver_read_raw(void);
uint16_t mux_adc_driver_read_pin(uint8_t pin);

#if !defined(ESP_PLATFORM)
void mux_adc_driver_set_mock_value(uint8_t channel, uint16_t raw_val);
void mux_adc_driver_set_mock_pin_value(uint8_t pin, uint16_t raw_val);
#endif

#ifdef __cplusplus
}
#endif
