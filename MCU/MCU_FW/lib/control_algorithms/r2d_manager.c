#include "r2d_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static r2d_state_t s_state = R2D_STATE_OFF;
static uint32_t s_sound_start_time = 0;

void r2d_manager_init(void) {
    s_state = R2D_STATE_OFF;
}

void r2d_manager_update(bool ts_active, bool brake_pressed, bool button_pressed) {
    if (!ts_active) {
        s_state = R2D_STATE_OFF;
        return;
    }
    
    switch (s_state) {
        case R2D_STATE_OFF:
            s_state = R2D_STATE_WAITING_BRAKE;
            break;
        case R2D_STATE_WAITING_BRAKE:
            if (brake_pressed) {
                s_state = R2D_STATE_WAITING_BUTTON;
            }
            break;
        case R2D_STATE_WAITING_BUTTON:
            if (!brake_pressed) {
                s_state = R2D_STATE_WAITING_BRAKE;
            } else if (button_pressed) {
                s_state = R2D_STATE_SOUNDING;
                s_sound_start_time = xTaskGetTickCount();
                // Play RTDS here (dummy)
            }
            break;
        case R2D_STATE_SOUNDING:
            // Formula Student rules require RTDS sound
            if ((xTaskGetTickCount() - s_sound_start_time) > pdMS_TO_TICKS(2000)) {
                s_state = R2D_STATE_READY;
            }
            break;
        case R2D_STATE_READY:
            // Car is ready to drive
            break;
    }
}

r2d_state_t r2d_manager_get_state(void) {
    return s_state;
}
