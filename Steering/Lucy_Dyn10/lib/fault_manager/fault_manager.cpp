#include "fault_manager.h"
#include <Arduino.h>

static bool s_high_fault_active = false;

void fault_manager_init(void) {
    s_high_fault_active = false;
}

void fault_manager_report(fault_category_t category, fault_priority_t priority, uint32_t code) {
    if (priority == FAULT_PRIORITY_HIGH) {
        s_high_fault_active = true;
        Serial.print("CRITICAL FAULT: Cat ");
        Serial.print((int)category);
        Serial.print(", Code ");
        Serial.println(code);
    } else {
        Serial.print("WARNING (LOW PRIORITY): Cat ");
        Serial.print((int)category);
        Serial.print(", Code ");
        Serial.println(code);
    }
}

bool fault_manager_is_high_fault_active(void) {
    return s_high_fault_active;
}

void fault_manager_clear_all(void) {
    s_high_fault_active = false;
}
