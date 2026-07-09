#include "r2d_manager.h"
#include "fault_manager.h"

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define GET_TICKS() xTaskGetTickCount()
#define TIME_TO_TICKS(ms) pdMS_TO_TICKS(ms)
#else
static uint32_t s_mock_tick = 0;
#define GET_TICKS() (++s_mock_tick)
#define TIME_TO_TICKS(ms) ((uint32_t)(ms))
#endif

static r2d_state_t s_state = R2D_STATE_OFF;
static uint32_t s_sound_start_time = 0;

void r2d_manager_init(void) {
    s_state = R2D_STATE_OFF;
}

void r2d_manager_update(bool ts_active, bool brake_pressed, bool button_pressed) {
    // SEGURIDAD: Si no hay TS o hay fallo crítico activo, R2D se bloquea permanentemente
    if (!ts_active || fault_manager_is_high_fault_active()) {
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
                s_sound_start_time = GET_TICKS();
            }
            break;
        case R2D_STATE_SOUNDING:
            // Formula Student rules require RTDS sound
            if ((GET_TICKS() - s_sound_start_time) > TIME_TO_TICKS(2000)) {
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
