#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ota_service_init(void);
void ota_set_r2d_state(bool is_r2d);

#ifdef __cplusplus
}
#endif
