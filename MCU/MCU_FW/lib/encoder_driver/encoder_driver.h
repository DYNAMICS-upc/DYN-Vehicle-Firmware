#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pines de los Encoders según mcu.ino (L37-L41)
#define PIN_ENC_FL 35
#define PIN_ENC_FR 47
#define PIN_ENC_RL 36
#define PIN_ENC_RR 48

// Constantes físicas del vehículo según mcu.ino (L44-L46)
#define WHEEL_RADIUS 0.27f        // Metros
#define ENCODER_FRONT_PPR 600.0f  // Pulsos por revolución encoder delantero
#define ENCODER_REAR_PPR 60.0f    // Pulsos por revolución encoder trasero

typedef struct {
    float rpm_fl;
    float rpm_fr;
    float rpm_rl;
    float rpm_rr;
    uint16_t speed_fl_cms;        // Velocidad rueda FL en cm/s (m/s * 100) para CAN 0x20
    uint16_t speed_fr_cms;        // Velocidad rueda FR en cm/s (m/s * 100) para CAN 0x20
    uint16_t speed_rl_cms;        // Velocidad rueda RL en cm/s (m/s * 100) para CAN 0x20
    uint16_t speed_rr_cms;        // Velocidad rueda RR en cm/s (m/s * 100) para CAN 0x20
    uint32_t speed_front_avg;     // Promedio delantero
    uint32_t speed_rear_avg;      // Promedio trasero
} wheel_speeds_t;

void encoder_driver_init(void);
void encoder_driver_update(void);
void encoder_driver_get_speeds(wheel_speeds_t* out);

#ifdef __cplusplus
}
#endif
