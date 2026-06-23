#include <unity.h>
#include "fan_driver.h"
#include "ota_service.h"

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

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_driver_scaling);
    RUN_TEST(test_fan_driver_bounds);
    RUN_TEST(test_ota_service_init_native);
    return UNITY_END();
}
