#include "nextion_driver.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

void nextion_driver_init(void) {
#ifdef ARDUINO
    // Usamos Serial1 del Mega 2560 para comunicarnos con Nextion a 9600 baudios (defecto de Nextion)
    Serial1.begin(9600);
#endif
}

void nextion_driver_init_dma(void) {
#ifdef ARDUINO
    // Configuración DMA (mock para Mega2560, real si se porta a otro MCU)
    Serial1.begin(115200); // Higher baudrate for DMA
#endif
}

void nextion_driver_send_cmd(const char* cmd) {
    if (!cmd) return;
#ifdef ARDUINO
    Serial1.print(cmd);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
#endif
}
