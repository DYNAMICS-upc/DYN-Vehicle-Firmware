#pragma once
#include <Arduino.h>

void nextion_driver_init(void);
void nextion_driver_send_cmd(const char* cmd);
