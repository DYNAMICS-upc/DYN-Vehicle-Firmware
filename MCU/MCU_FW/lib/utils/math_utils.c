#include "math_utils.h"

uint32_t math_clamp(uint32_t value, uint32_t min, uint32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int32_t math_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if (in_max == in_min) return out_min; // Avoid division by zero
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
