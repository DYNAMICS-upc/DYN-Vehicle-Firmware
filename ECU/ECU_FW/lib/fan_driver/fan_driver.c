#include "fan_driver.h"

// Hardware dependencies mapped for ESP32 and native
#if defined(ESP_PLATFORM)
// ESP-IDF headers could go here. For the prototype we mock internally.
#endif

static uint8_t s_pin = 0;

#if !defined(ESP_PLATFORM)
// Mocks for native testing
static uint8_t s_mock_speed = 0;
uint8_t fan_driver_get_mock_speed(void) {
    return s_mock_speed;
}
#endif

void fan_driver_init(uint8_t pin) {
    s_pin = pin;
}

void fan_driver_set_speed(uint8_t duty_cycle) {
#if defined(ESP_PLATFORM)
    // Prototype: do nothing physically yet
    (void)duty_cycle;
#else
    s_mock_speed = duty_cycle;
#endif
}
