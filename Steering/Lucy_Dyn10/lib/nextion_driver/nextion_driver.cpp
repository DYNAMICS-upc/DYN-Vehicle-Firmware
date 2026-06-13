#include "nextion_driver.h"
#include <Arduino_FreeRTOS.h>
#include <task.h>

static dashboard_struct_t s_cached_dash;

void nextion_task(void *pvParameters) {
    (void)pvParameters;
    Serial2.begin(9600);
    dashboard_struct_init(&s_cached_dash);
    // Fuerzo estado diferente para que pinte la primera vez
    s_cached_dash.precharge_state = 255;
    
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

void nextion_driver_update(const dashboard_struct_t* dash) {
    if (dash->precharge_state != s_cached_dash.precharge_state) {
        s_cached_dash.precharge_state = dash->precharge_state;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "tPrecharge.txt=\"State: %d\"", dash->precharge_state);
        nextion_driver_send_cmd(cmd);
    }
    
    if (dash->speed_kmh != s_cached_dash.speed_kmh) {
        s_cached_dash.speed_kmh = dash->speed_kmh;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "tSpeed.txt=\"%d\"", dash->speed_kmh);
        nextion_driver_send_cmd(cmd);
    }
    
    if (dash->soc_percent != s_cached_dash.soc_percent) {
        s_cached_dash.soc_percent = dash->soc_percent;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "jSoc.val=%d", dash->soc_percent);
        nextion_driver_send_cmd(cmd);
    }

    if (dash->motor_temp != s_cached_dash.motor_temp) {
        s_cached_dash.motor_temp = dash->motor_temp;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "tMotorTemp.txt=\"%d C\"", dash->motor_temp);
        nextion_driver_send_cmd(cmd);
    }
    
    if (dash->hv_voltage != s_cached_dash.hv_voltage) {
        s_cached_dash.hv_voltage = dash->hv_voltage;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "tHVVolt.txt=\"%d V\"", dash->hv_voltage);
        nextion_driver_send_cmd(cmd);
    }

    if (dash->is_r2d != s_cached_dash.is_r2d) {
        s_cached_dash.is_r2d = dash->is_r2d;
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "tR2D.txt=\"%s\"", dash->is_r2d ? "R2D" : "OFF");
        nextion_driver_send_cmd(cmd);
    }
}
