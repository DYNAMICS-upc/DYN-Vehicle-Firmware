#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pines de Actuadores y Sensores (ecu.ino)
#define PIN_FAN_MOTOR       13
#define PIN_FAN_INV         14

#define CAN_ID_TEMPS        10    // 0x0A - 1 Hz (DLC 4)
#define CAN_ID_STS          11    // 0x0B - 100 Hz (DLC 8)
#define CAN_ID_ECU_DIAG_DTC 0x503 // 10 Hz (DLC 8)

// Utilidades Globales
#define EMA_FILTER_SHIFT(old_val, new_val, shift) ((((old_val) * ((1 << (shift)) - 1)) + (new_val)) >> (shift))

void bsp_init(void);

#ifdef __cplusplus
}
#endif
