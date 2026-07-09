#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_ID_STEERING_DIAGNOSTIC_DTC 0x504 // ID Diagnóstica segura no colisionante

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

#define FAULT_CODE_NEXTION_UART_ERR    301
#define FAULT_CODE_ROTARY_ENCODER_ERR  302
#define FAULT_CODE_STEERING_TWAI_ERR   303

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

// Diagnostic State for CAN & Telemetry
fault_record_t fault_manager_get_last_fault(void);

#ifdef __cplusplus
}
#endif
