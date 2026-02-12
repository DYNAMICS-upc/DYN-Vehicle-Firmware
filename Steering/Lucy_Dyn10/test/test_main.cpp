#include <unity.h>

void setUp(void) {}
void tearDown(void) {}

// Simularemos la lógica del loop()
bool read_raw_button(int state) {
    return (state == 0); // LOW = true (pressed)
}

void test_button_logic_pressed(void) {
    TEST_ASSERT_EQUAL(true, read_raw_button(0));
}

void test_button_logic_released(void) {
    TEST_ASSERT_EQUAL(false, read_raw_button(1));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_button_logic_pressed);
    RUN_TEST(test_button_logic_released);
    return UNITY_END();
}
