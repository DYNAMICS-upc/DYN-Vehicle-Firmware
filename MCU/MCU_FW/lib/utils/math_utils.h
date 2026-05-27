#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t math_clamp(uint32_t value, uint32_t min, uint32_t max);
int32_t math_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

#ifdef __cplusplus
}
#endif
