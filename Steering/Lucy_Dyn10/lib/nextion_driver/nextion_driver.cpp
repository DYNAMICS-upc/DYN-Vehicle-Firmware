#include "nextion_driver.h"
#include <Arduino_FreeRTOS.h>
#include <task.h>

void nextion_task(void *pvParameters) {
    (void)pvParameters;
    Serial2.begin(9600);
    while (1) {
        // Enviar un keep-alive basico o comandos cacheados
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void nextion_driver_init(void) {
    xTaskCreate(nextion_task, "Nextion", 128, NULL, 1, NULL);
}

void nextion_driver_send_cmd(const char* cmd) {
    Serial2.print(cmd);
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
}
