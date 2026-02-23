#include <unity.h>
#include "button_driver.h"

static int mock_pin_state = 1; // 1 = HIGH, 0 = LOW
static bool press_called = false;
static bool release_called = false;

// Mock implementations for native environment
extern "C" {
    int digitalRead(uint8_t pin) {
        (void)pin;
        return mock_pin_state;
    }
    uint32_t millis(void) {
        return 0; // Not used directly when mock time is injected
    }
}

void on_press() { press_called = true; }
void on_release() { release_called = true; }

void setUp(void) {
    mock_pin_state = 1;
    press_called = false;
    release_called = false;
    button_driver_set_time(0);
    button_driver_init(2, on_press, on_release);
}

void tearDown(void) {}

void test_debounce_ignore_noise(void) {
    mock_pin_state = 0; // Press
    button_driver_update();
    button_driver_set_time(10);
    button_driver_update();
    
    mock_pin_state = 1; // Release (Noise)
    button_driver_update();
    button_driver_set_time(20);
    button_driver_update();
    
    TEST_ASSERT_FALSE(press_called);
}

void test_debounce_valid_press(void) {
    mock_pin_state = 0; // Press
    button_driver_update();
    
    button_driver_set_time(60); // After debounce
    button_driver_update();
    
    TEST_ASSERT_TRUE(press_called);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_debounce_ignore_noise);
    RUN_TEST(test_debounce_valid_press);
    return UNITY_END();
}
