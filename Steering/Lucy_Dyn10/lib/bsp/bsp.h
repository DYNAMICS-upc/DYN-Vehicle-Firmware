#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_PIN_CAN_CS 10
#define BSP_PIN_BTN_LAUNCH 2
#define BSP_PIN_LED_R2D 8
#define BSP_PIN_LED_FAULT 9
#define BSP_PIN_LED_HEARTBEAT 13

void bsp_steering_init(void);

#ifdef __cplusplus
}
#endif
