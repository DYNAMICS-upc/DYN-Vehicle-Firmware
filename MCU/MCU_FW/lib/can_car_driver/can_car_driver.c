#include "can_car_driver.h"
#include <string.h>

static car_rx_data_t s_mock_data;

void can_car_init(void) {
    memset(&s_mock_data, 0, sizeof(s_mock_data));
}

bool can_car_rx_available(uint32_t wait_ms) {
    // Simular que siempre hay datos o no hay
    (void)wait_ms;
    return false;
}

void can_car_drain_rx(car_rx_data_t *out) {
    if (out) {
        *out = s_mock_data;
    }
}

void can_car_send_frame(const car_tx_frame_t *f) {
    // Simular envío
    (void)f;
}
