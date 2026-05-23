#include <Arduino.h>
#include "buttons_app.h"
#include "button_driver.h"
#include "ipc.h"

const int BTN_PIN = 2;

static void on_btn_press() {
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = true;
        ipc_send_state(&state);
    }
}

static void on_btn_release() {
    volante_state_t state;
    if (ipc_receive_state(&state)) {
        state.btn_launch_pressed = false;
        ipc_send_state(&state);
    }
}

void buttons_app_init(void) {
    pinMode(BTN_PIN, INPUT_PULLUP);
    button_driver_init(BTN_PIN, on_btn_press, on_btn_release);
}

void buttons_app_update(void) {
    button_driver_update();
}
