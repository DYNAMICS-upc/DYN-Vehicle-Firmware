#include <unity.h>
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

// Simulated debounce logic
static uint32_t simulated_millis = 0;
static uint32_t last_debounce_time = 0;
static const uint32_t DEBOUNCE_DELAY = 50;
static int last_btn_state = 1;
static int btn_state = 1;
static bool led_on = false;

void simulate_loop(int reading) {
    if (reading != last_btn_state) {
        last_debounce_time = simulated_millis;
    }

    if ((simulated_millis - last_debounce_time) > DEBOUNCE_DELAY) {
        if (reading != btn_state) {
            btn_state = reading;
            led_on = (btn_state == 0);
        }
    }
    last_btn_state = reading;
}

void test_debounce_ignore_noise(void) {
    // Reset state
    simulated_millis = 0;
    last_debounce_time = 0;
    last_btn_state = 1;
    btn_state = 1;
    led_on = false;

    // Simulate noise (press for 10ms, then release)
    simulate_loop(0);
    simulated_millis += 10;
    simulate_loop(0);
    
    // Noise ends
    simulate_loop(1);
    simulated_millis += 10;
    simulate_loop(1);
    
    // Check state (should still be false)
    TEST_ASSERT_EQUAL(false, led_on);
}

void test_debounce_valid_press(void) {
    // Simulate valid press for 60ms
    simulate_loop(0);
    simulated_millis += 60;
    simulate_loop(0);
    
    // Check state (should be true)
    TEST_ASSERT_EQUAL(true, led_on);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_debounce_ignore_noise);
    RUN_TEST(test_debounce_valid_press);
    return UNITY_END();
}
