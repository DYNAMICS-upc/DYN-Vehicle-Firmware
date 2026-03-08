#include <unity.h>
#include "apps_driver.h"

void setUp(void) {
    apps_driver_init(1, 2);
}
void tearDown(void) {}

void test_apps_plausible(void) {
    apps_driver_set_mock(400, 200); // 400 == 200 * 2
    uint16_t val;
    TEST_ASSERT_TRUE(apps_driver_read(&val));
    TEST_ASSERT_EQUAL(400, val);
}

void test_apps_implausible(void) {
    apps_driver_set_mock(400, 100); // 400 != 100 * 2 (out of bounds)
    uint16_t val;
    TEST_ASSERT_FALSE(apps_driver_read(&val));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_apps_plausible);
    RUN_TEST(test_apps_implausible);
    return UNITY_END();
}
