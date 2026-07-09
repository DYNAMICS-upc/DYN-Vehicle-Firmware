#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pines ADS8688 según ecu.ino
#define ADC_SCLK_PIN  41
#define ADC_MISO_PIN  40
#define ADC_MOSI_PIN  38
#define ADC_CS_PIN    42
#define ADC_RST_PIN   39

// Canales de Galgas STS según ecu.ino
#define CH_STSRR 1   // Suspensión trasera derecha
#define CH_STSRL 2   // Suspensión trasera izquierda
#define CH_STSFR 3   // Suspensión delantera derecha
#define CH_STSFL 4   // Suspensión delantera izquierda

#define CH_TEMP_MOTOR 0
#define CH_TEMP_INV   7

void ads8688_driver_init(void);
bool ads8688_driver_read_raw(uint8_t channel, uint16_t* out_raw);
bool ads8688_driver_read_temp(uint8_t channel, double* out_temp_c);
float ads8688_driver_bosch_r2t(float r_ohm);

#ifdef __cplusplus
}
#endif
