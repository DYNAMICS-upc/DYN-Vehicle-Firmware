#include "fault_manager.h"
#include <stdio.h>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
static const char *TAG = "FAULT_MGR";
#endif

static bool s_high_fault_active = false;

void fault_manager_init(void) {
    s_high_fault_active = false;
}

void fault_manager_report(fault_category_t category, fault_priority_t priority, uint32_t code) {
    if (priority == FAULT_PRIORITY_HIGH) {
        s_high_fault_active = true;
#if defined(ESP_PLATFORM)
        ESP_LOGE(TAG, "CRITICAL FAULT: Cat %d, Code %lu", (int)category, (unsigned long)code);
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

void fault_manager_clear_all(void) {
    s_high_fault_active = false;
}
