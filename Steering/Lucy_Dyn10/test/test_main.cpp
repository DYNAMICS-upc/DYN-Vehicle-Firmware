#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

// Un test real (dummy por ahora, pero validará la compilación de test)
// Simularemos el estado de un LED
bool led_state = false;
void toggle_led() { led_state = !led_state; }

void test_led_toggle(void) {
    led_state = false;
    toggle_led();
    TEST_ASSERT_EQUAL(true, led_state);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_led_toggle);
    return UNITY_END();
}
