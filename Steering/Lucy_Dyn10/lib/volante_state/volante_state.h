#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Struct to unify all the steering wheel state data
typedef struct {
    // Buttons state
    bool btn_drs_pressed;
    bool btn_launch_pressed;
    bool btn_neutral_pressed;
    
    // Rotaries state
    uint8_t rotary_map_val;
    uint8_t rotary_tc_val;
    
    // UI and display state
    uint8_t current_page;
    bool is_r2d;
    bool has_bms_fault;
    bool has_imd_fault;
} volante_state_t;

#ifdef __cplusplus
}
#endif
