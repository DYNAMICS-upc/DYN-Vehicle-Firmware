#include "mosfet_driver.h"
#include "fault_manager.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#include "esp_log.h"
static const char *TAG = "MOSFET_DRV";
#endif

static const uint8_t s_mosfet_pins[MUX_CHANNELS] = { 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 41, 42 };
static uint8_t s_mosfet_status[MUX_CHANNELS] = { 0 };

void mosfet_driver_init(void) {
    for (int i = 0; i < MUX_CHANNELS; i++) {
        uint8_t pin = s_mosfet_pins[i];
#if defined(ESP_PLATFORM)
        gpio_reset_pin((gpio_num_t)pin);
        gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)pin, 0); // LOW turns ON in pdm.ino
#else
        (void)pin;
#endif
        s_mosfet_status[i] = 1; // Default to ON / active
    }
}

void mosfet_driver_set_channel(uint8_t channel, bool enable) {
    if (channel >= MUX_CHANNELS) return;
    
    // SEGURIDAD: Si el canal fue bloqueado por fallo previo, rechazar cualquier intento de reactivación
    if (enable && fault_manager_is_channel_locked(channel)) {
#if defined(ESP_PLATFORM)
        ESP_LOGE(TAG, "BLOCKED: Attempt to re-enable locked channel %d rejected by Safety Layer!", channel);
#endif
        s_mosfet_status[channel] = 0;
        return;
    }

    uint8_t pin = s_mosfet_pins[channel];
    if (enable) {
#if defined(ESP_PLATFORM)
        gpio_set_level((gpio_num_t)pin, 0); // LOW = ON
#else
        (void)pin;
#endif
        s_mosfet_status[channel] = 1;
    } else {
#if defined(ESP_PLATFORM)
        gpio_set_level((gpio_num_t)pin, 1); // HIGH = OFF / Trip
#else
        (void)pin;
#endif
        s_mosfet_status[channel] = 0;
    }
}

void mosfet_driver_set_all(bool enable) {
    for (int i = 0; i < MUX_CHANNELS; i++) {
        mosfet_driver_set_channel(i, enable);
    }
}

uint8_t mosfet_driver_get_status(uint8_t channel) {
    if (channel >= MUX_CHANNELS) return 0;
    return s_mosfet_status[channel];
}

void mosfet_driver_get_all_statuses(uint8_t *out_statuses) {
    if (!out_statuses) return;
    memcpy(out_statuses, s_mosfet_status, sizeof(s_mosfet_status));
}
