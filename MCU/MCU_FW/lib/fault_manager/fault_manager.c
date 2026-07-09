#include "fault_manager.h"
#include <stdio.h>
#include <string.h>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
static const char *TAG = "MCU_FAULT_MGR";
#endif

static bool s_high_fault_active = false;
static uint32_t s_locked_subsystems = 0;
static fault_record_t s_last_fault = {};

void fault_manager_init(void) {
    s_high_fault_active = false;
    s_locked_subsystems = 0;
    memset(&s_last_fault, 0, sizeof(s_last_fault));
}

void fault_manager_report(fault_category_t category, fault_priority_t priority, uint32_t code) {
    s_last_fault.active = true;
    s_last_fault.category = category;
    s_last_fault.priority = priority;
    s_last_fault.code = code;
    s_last_fault.fault_count++;

    if (priority == FAULT_PRIORITY_HIGH) {
        s_high_fault_active = true;
#if defined(ESP_PLATFORM)
        ESP_LOGE(TAG, "MCU CRITICAL FAULT: Cat %d, Code %lu (Total Faults: %lu)", 
                 (int)category, (unsigned long)code, (unsigned long)s_last_fault.fault_count);
#endif
    } else {
#if defined(ESP_PLATFORM)
        ESP_LOGW(TAG, "MCU WARNING (LOW PRIORITY): Cat %d, Code %lu", (int)category, (unsigned long)code);
#endif
    }
}

bool fault_manager_is_high_fault_active(void) {
    return s_high_fault_active;
}

void fault_manager_lock_subsystem(uint32_t subsys_mask) {
    s_locked_subsystems |= subsys_mask;
    s_high_fault_active = true;
#if defined(ESP_PLATFORM)
    ESP_LOGE(TAG, "MCU SUBSYSTEM 0x%02lX LOCKED TO PREVENT UNSAFE RE-ARMING", (unsigned long)subsys_mask);
#endif
}

bool fault_manager_is_subsystem_locked(uint32_t subsys_mask) {
    return (s_locked_subsystems & subsys_mask) != 0;
}

uint32_t fault_manager_get_locked_subsystems(void) {
    return s_locked_subsystems;
}

fault_record_t fault_manager_get_last_fault(void) {
    return s_last_fault;
}

void fault_manager_clear_all(void) {
    s_high_fault_active = false;
    s_locked_subsystems = 0;
    s_last_fault.active = false;
}
