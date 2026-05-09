#ifndef APP_LOG_H
#define APP_LOG_H

#include "esp_log.h"

// Define APP_VERBOSE_LEVEL based on build configuration
// Level 0: No logs (or only critical errors if desired, currently disables all if set so)
// Level 1: Logs only task execution
// Level 2: Level 1 + dispatch executions and values
// Level 3: Level 2 + every message that arrives (raw messages)
#ifndef APP_VERBOSE_LEVEL
  #ifdef APP_DEBUG_MODE
    #if APP_DEBUG_MODE
      #define APP_VERBOSE_LEVEL 1 // Default to 1 if legacy debug mode is on
    #else
      #define APP_VERBOSE_LEVEL 0
    #endif
  #else
    #define APP_VERBOSE_LEVEL 0  // Default to release mode (no logs)
  #endif
#endif

// APP_LOG macros: conditionally compile based on APP_VERBOSE_LEVEL
#if APP_VERBOSE_LEVEL > 0
#define APP_LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define APP_LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
// We map APP_LOGI to ESP_LOGI only if level is greater than 3, so standard info logs don't pollute verbose 1-3. 
// Or you can map it to ESP_LOGI unconditionally if you want generic info messages too.
#define APP_LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define APP_LOGE(tag, format, ...) do { if (0) { ESP_LOGE(tag, format, ##__VA_ARGS__); } } while (0)
#define APP_LOGW(tag, format, ...) do { if (0) { ESP_LOGW(tag, format, ##__VA_ARGS__); } } while (0)
#define APP_LOGI(tag, format, ...) do { if (0) { ESP_LOGI(tag, format, ##__VA_ARGS__); } } while (0)
#endif

// Custom log macros for semantic levels
#if APP_VERBOSE_LEVEL >= 1
#define APP_LOG_TASK(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define APP_LOG_TASK(tag, format, ...) do { if (0) { ESP_LOGI(tag, format, ##__VA_ARGS__); } } while (0)
#endif

#if APP_VERBOSE_LEVEL >= 2
#define APP_LOG_DISPATCH(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define APP_LOG_DISPATCH(tag, format, ...) do { if (0) { ESP_LOGI(tag, format, ##__VA_ARGS__); } } while (0)
#endif

#if APP_VERBOSE_LEVEL >= 3
#define APP_LOG_MSG(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define APP_LOG_MSG(tag, format, ...) do { if (0) { ESP_LOGI(tag, format, ##__VA_ARGS__); } } while (0)
#endif


#endif // APP_LOG_H
