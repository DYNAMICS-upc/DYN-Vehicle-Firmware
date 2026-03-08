#include <unity.h>
#include "mosfet_driver.h"

void setUp(void) {
    mosfet_driver_init(1, 2);
}
void tearDown(void) {}

void test_mosfet_no_fault(void) {
    mosfet_driver_set_mock_current(500);
    TEST_ASSERT_FALSE(mosfet_driver_check_fault());
}

void test_mosfet_fault(void) {
    mosfet_driver_set_mock_current(900); // Exceeds 800
    TEST_ASSERT_TRUE(mosfet_driver_check_fault());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_mosfet_no_fault);
    RUN_TEST(test_mosfet_fault);
    return UNITY_END();
}
