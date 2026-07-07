#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pdm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void can_service_init(void);
void can_service_send_all_telemetry(const uint8_t *mosfets_status, const uint16_t *consumos_can, float v_bat_actual);
void can_service_check_alerts(void);

#ifdef __cplusplus
}
#endif
