#include "launch_ctrl.h"

#define TARGET_SLIP_RATIO 1150 // 1.15 * 1000
#define MAX_TORQUE_MULTIPLIER 1000

void launch_ctrl_init(void) {
    // Initialization of launch control states
}

int32_t launch_ctrl_update(uint32_t speed_front, uint32_t speed_rear) {
    // Integer-only slip ratio calculation
    if (speed_front < 10) {
        // Al estar parados, permitimos todo el par
        return MAX_TORQUE_MULTIPLIER;
    }

    // Slip actual (rear / front) multiplicado por 1000 para precision
    uint32_t current_slip = (speed_rear * 1000) / speed_front;

    if (current_slip > TARGET_SLIP_RATIO) {
        // Exceso de slip, reducimos par
        int32_t reduction = (current_slip - TARGET_SLIP_RATIO) * 2; // Kp dummy
        int32_t multiplier = MAX_TORQUE_MULTIPLIER - reduction;
        if (multiplier < 100) {
            multiplier = 100; // Minimo 10% de par
        }
        return multiplier;
    }

    return MAX_TORQUE_MULTIPLIER;
}
