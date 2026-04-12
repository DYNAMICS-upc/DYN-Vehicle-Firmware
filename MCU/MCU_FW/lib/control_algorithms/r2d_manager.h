#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    R2D_STATE_OFF = 0,
    R2D_STATE_WAITING_BRAKE,
    R2D_STATE_WAITING_BUTTON,
    R2D_STATE_SOUNDING,
    R2D_STATE_READY
} r2d_state_t;

void r2d_manager_init(void);
void r2d_manager_update(bool ts_active, bool brake_pressed, bool button_pressed);
r2d_state_t r2d_manager_get_state(void);

#ifdef __cplusplus
}
#endif
