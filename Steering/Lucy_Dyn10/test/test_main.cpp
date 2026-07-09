#include <unity.h>
#include "can_driver.h"

void setUp(void) {}
void tearDown(void) {}

void test_can_driver_invalid_payload(void) {
    can_driver_init(10);
    bool res = can_driver_send_frame(0x100, NULL, 0);
    TEST_ASSERT_FALSE(res);
}

void test_can_driver_valid_payload(void) {
    can_driver_init(10);
    uint8_t data[4] = {0x11, 0x22, 0x33, 0x44};
    bool res = can_driver_send_frame(0x200, data, 4);
    TEST_ASSERT_TRUE(res);
    TEST_ASSERT_EQUAL_HEX32(0x200, can_driver_get_last_id());
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_can_driver_invalid_payload);
    RUN_TEST(test_can_driver_valid_payload);
    return UNITY_END();
}
