#include <unity.h>
#include "fan_driver.h"

void setUp(void) {}
void tearDown(void) {}

void test_fan_driver_scaling(void) {
    fan_driver_init(9);
    fan_driver_set_speed(128);
    TEST_ASSERT_EQUAL(128, fan_driver_get_mock_speed());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_driver_scaling);
    return UNITY_END();
}
