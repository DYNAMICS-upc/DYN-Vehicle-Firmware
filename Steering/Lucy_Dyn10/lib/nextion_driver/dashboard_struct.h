#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // Vehículo
    uint8_t speed_kmh;
    
    // Batería
    uint8_t soc_percent;
    uint16_t hv_voltage;
    int16_t hv_current;
    
    // Temperaturas
    uint8_t motor_temp;
    uint8_t inv_temp;
    uint8_t bat_max_temp;
    
    // Modos
    uint8_t power_map;
    uint8_t tc_map;
    
    // Estados y fallos
    bool is_r2d;
    bool bms_fault;
    bool imd_fault;
    bool inv_fault;
    uint8_t precharge_state;
} dashboard_struct_t;

void dashboard_struct_init(dashboard_struct_t* dash);

#ifdef __cplusplus
}
#endif
