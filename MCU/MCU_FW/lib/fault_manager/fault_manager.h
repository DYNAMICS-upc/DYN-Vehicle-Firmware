#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define FAULT_SUBSYS_APPS           (1 << 0)
#define FAULT_SUBSYS_BRAKES         (1 << 1)
#define FAULT_SUBSYS_INVERTER_TWAI  (1 << 2)
#define FAULT_SUBSYS_CAN_CAR        (1 << 3)
#define FAULT_SUBSYS_BMS_SAG        (1 << 4)

#define FAULT_CODE_APPS_IMPLAUSIBLE 101
#define FAULT_CODE_APPS_WIRE_BREAK  102
#define FAULT_CODE_BRAKE_SENSOR_ERR 103
#define FAULT_CODE_BRAKE_DISCONNECT 103
#define FAULT_CODE_TWAI_BUS_OFF     104
#define FAULT_CODE_BMS_SAG_LIMIT    105
#define FAULT_CODE_BSPD_TRIPPED     106

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

// Subsystem Locking & Safe State Latch
void fault_manager_lock_subsystem(uint32_t subsys_mask);
bool fault_manager_is_subsystem_locked(uint32_t subsys_mask);
uint32_t fault_manager_get_locked_subsystems(void);

// Diagnostic State for CAN & Telemetry
fault_record_t fault_manager_get_last_fault(void);

#ifdef __cplusplus
}
#endif
