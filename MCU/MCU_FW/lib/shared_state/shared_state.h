#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t r2d_state;
    bool ts_active;
    bool brake_pressed;
    bool r2d_button_pressed;
    bool implausibility_fault;
} mcu_shared_state_t;

void shared_state_init(void);
void shared_state_set(const mcu_shared_state_t* state);
void shared_state_get(mcu_shared_state_t* state);

#ifdef __cplusplus
}
#endif
