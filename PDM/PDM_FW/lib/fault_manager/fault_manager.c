#include "fault_manager.h"
#include <stdio.h>
#include <string.h>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
static const char *TAG = "FAULT_MGR";
#endif

static bool s_high_fault_active = false;
static uint16_t s_locked_channels_mask = 0;
static fault_record_t s_last_fault = {};

void fault_manager_init(void) {
    s_high_fault_active = false;
    s_locked_channels_mask = 0;
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
        ESP_LOGE(TAG, "CRITICAL FAULT: Cat %d, Code %lu (Total Faults: %lu)", 
                 (int)category, (unsigned long)code, (unsigned long)s_last_fault.fault_count);
#endif
    } else {
#if defined(ESP_PLATFORM)
        ESP_LOGW(TAG, "WARNING (LOW PRIORITY): Cat %d, Code %lu", (int)category, (unsigned long)code);
#endif
    }
}

bool fault_manager_is_high_fault_active(void) {
    return s_high_fault_active;
}

void fault_manager_lock_channel(uint8_t channel) {
    if (channel < 16) {
        s_locked_channels_mask |= (1 << channel);
#if defined(ESP_PLATFORM)
        ESP_LOGE(TAG, "CHANNEL %d PERMANENTLY LOCKED DUE TO FAULT. RE-ENABLE ATTEMPTS BLOCKED.", channel);
#endif
    }
}

bool fault_manager_is_channel_locked(uint8_t channel) {
    if (channel < 16) {
        return (s_locked_channels_mask & (1 << channel)) != 0;
    }
    return true;
}

uint16_t fault_manager_get_locked_mask(void) {
    return s_locked_channels_mask;
}

fault_record_t fault_manager_get_last_fault(void) {
    return s_last_fault;
}

void fault_manager_clear_all(void) {
    s_high_fault_active = false;
    s_locked_channels_mask = 0;
    s_last_fault.active = false;
}
