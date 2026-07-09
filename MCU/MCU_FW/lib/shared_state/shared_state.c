#include "shared_state.h"
#include <string.h>

static mcu_shared_state_t s_state;

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static StaticSemaphore_t s_mutex_buffer;
static SemaphoreHandle_t s_mutex = NULL;

void shared_state_init(void) {
    memset(&s_state, 0, sizeof(s_state));
    s_mutex = xSemaphoreCreateMutexStatic(&s_mutex_buffer);
}

void shared_state_set(const mcu_shared_state_t* state) {
    if (s_mutex && xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(&s_state, state, sizeof(mcu_shared_state_t));
        xSemaphoreGive(s_mutex);
    }
}

void shared_state_get(mcu_shared_state_t* state) {
    if (s_mutex && xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(state, &s_state, sizeof(mcu_shared_state_t));
        xSemaphoreGive(s_mutex);
    }
}
#else
void shared_state_init(void) {
    memset(&s_state, 0, sizeof(s_state));
}
void shared_state_set(const mcu_shared_state_t* state) {
    if (state) memcpy(&s_state, state, sizeof(mcu_shared_state_t));
}
void shared_state_get(mcu_shared_state_t* state) {
    if (state) memcpy(state, &s_state, sizeof(mcu_shared_state_t));
}
#endif
