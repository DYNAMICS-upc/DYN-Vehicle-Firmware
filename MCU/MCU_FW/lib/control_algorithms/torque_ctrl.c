#include "torque_ctrl.h"
#include "math_utils.h"

#define MAX_TORQUE 32767
#define THROTTLE_MAX 1000

void torque_ctrl_init(void) {
    // Initialization for torque control
}

int32_t torque_ctrl_calculate(uint32_t throttle_raw, uint32_t speed_rpm, bool brake_pressed, bool r2d_active, int32_t slip_multiplier) {
    if (!r2d_active) {
        return 0; // No torque if not ready to drive
    }

    if (brake_pressed) {
        return 0; // Cut torque if brake is pressed (safety)
    }

    throttle_raw = math_clamp(throttle_raw, 0, THROTTLE_MAX);

    int32_t torque = math_map(throttle_raw, 0, THROTTLE_MAX, 0, MAX_TORQUE);
    
    // Aplicar reduccion por excesivo slip (slip_multiplier va de 100 a 1000)
    torque = (torque * slip_multiplier) / 1000;

    // Simple anti-kick logic using integers
    if (speed_rpm < 50 && torque > (MAX_TORQUE / 10)) {
        torque = MAX_TORQUE / 10; // Limit torque at very low speeds
    }

    return torque;
}
