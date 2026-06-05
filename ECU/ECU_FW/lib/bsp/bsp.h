#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// --- Macros Globales ECU ---
#define ECU_PIN_FAN         18
#define ECU_CAN_ID_SENSORS  0x400

// --- Utilidades Globales ---
// Filtro EMA (Exponential Moving Average)
#define EMA_FILTER_SHIFT(old_val, new_val, shift) ((((old_val) * ((1 << (shift)) - 1)) + (new_val)) >> (shift))

void bsp_init(void);

#ifdef __cplusplus
}
#endif
