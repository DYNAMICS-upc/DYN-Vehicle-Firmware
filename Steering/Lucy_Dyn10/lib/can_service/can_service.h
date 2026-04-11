#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void can_service_init(void);
void can_service_update(void);

#ifdef __cplusplus
}
#endif
