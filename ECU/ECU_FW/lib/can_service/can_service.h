#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_ID_ECU_DIAGNOSTIC_DTC 0x503 // ID Diagnóstica segura no colisionante

void can_service_init(void);
void can_service_log(const char* msg);
void can_service_send_diagnostic_dtc(uint8_t fan_motor_pct, uint8_t fan_inv_pct);

#ifdef __cplusplus
}
#endif
