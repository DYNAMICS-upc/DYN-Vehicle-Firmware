#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_ID_ECU_DIAGNOSTIC_DTC 0x503 // ID Diagnóstica segura no colisionante

typedef enum {
    FAULT_PRIORITY_LOW = 0,
    FAULT_PRIORITY_HIGH = 1
} fault_priority_t;

typedef enum {
    FAULT_CAT_HARDWARE = 0,
    FAULT_CAT_COMMUNICATION,
    FAULT_CAT_RESOURCES,
    FAULT_CAT_TIMING
} fault_category_t;

#define FAULT_CODE_MOTOR_NTC_FAIL   201
#define FAULT_CODE_INV_NTC_FAIL     202
#define FAULT_CODE_ADS8688_SPI_ERR  203
#define FAULT_CODE_TWAI_BUS_OFF     204

typedef struct {
    bool active;
    fault_category_t category;
    fault_priority_t priority;
    uint32_t code;
    uint32_t fault_count;
} fault_record_t;

void fault_manager_init(void);
void fault_manager_report(fault_category_t category, fault_priority_t priority, uint32_t code);
bool fault_manager_is_high_fault_active(void);
void fault_manager_clear_all(void);

// Failsafe State Latch
void fault_manager_set_failsafe(bool active);
bool fault_manager_is_failsafe_active(void);

// Diagnostic State for CAN & Telemetry
fault_record_t fault_manager_get_last_fault(void);

#ifdef __cplusplus
}
#endif
