#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_SDCB        13
#define PIN_DASHB       14
#define PIN_R2D_BUZZER  1
#define PIN_BRAKE_LIGHT 21
#define PIN_STEERING    17

#define PIN_EXT1        15
#define PIN_EXT2        16
#define PIN_HPS_FRONT   8
#define PIN_HPS_REAR    18

void bsp_init(void);
bool bsp_read_ts_active(void);
bool bsp_read_dash_button(void);
void bsp_set_r2d_buzzer(bool active);
void bsp_set_brake_light(bool active);
int16_t bsp_read_steering_angle(void);

#ifdef __cplusplus
}
#endif
