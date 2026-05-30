#pragma once
#include <Arduino.h>
#include "dashboard_struct.h"

void nextion_driver_init(void);
void nextion_driver_send_cmd(const char* cmd);
void nextion_driver_update(const dashboard_struct_t* dash);
