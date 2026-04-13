#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mux_adc_driver_init(uint8_t pin_s0, uint8_t pin_s1, uint8_t pin_s2, uint8_t adc_pin);
void mux_adc_driver_select(uint8_t channel);
uint16_t mux_adc_driver_read(void);

#ifdef __cplusplus
}
#endif
