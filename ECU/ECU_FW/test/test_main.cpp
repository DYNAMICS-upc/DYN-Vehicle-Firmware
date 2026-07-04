#include <unity.h>
#include "fan_driver.h"
#include "fan_driver.h"
#include "ota_service.h"
#include "fault_manager.h"

void setUp(void) {}
void tearDown(void) {}

void test_fan_driver_scaling(void) {
    fan_driver_init(9);
    fan_driver_set_speed(128);
    TEST_ASSERT_EQUAL(128, fan_driver_get_mock_speed());
}

void test_fan_driver_bounds(void) {
    fan_driver_set_speed(255);
    TEST_ASSERT_EQUAL(255, fan_driver_get_mock_speed());
    fan_driver_set_speed(0);
    TEST_ASSERT_EQUAL(0, fan_driver_get_mock_speed());
}

void test_ota_service_init_native(void) {
    ota_service_init(); // Should safely do nothing in native env
    TEST_ASSERT_TRUE(true);
}

void test_fault_manager(void) {
    fault_manager_init();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // Low priority should not trigger high fault state
    fault_manager_report(FAULT_CAT_RESOURCES, FAULT_PRIORITY_LOW, 1);
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // High priority should lock the system
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 2);
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    
    fault_manager_clear_all();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_driver_scaling);
    RUN_TEST(test_fan_driver_bounds);
    RUN_TEST(test_fan_driver_bounds);
    RUN_TEST(test_ota_service_init_native);
    RUN_TEST(test_fault_manager);
    return UNITY_END();
}
