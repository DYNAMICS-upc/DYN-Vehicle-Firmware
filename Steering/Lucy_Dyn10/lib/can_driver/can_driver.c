#include "can_driver.h"
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>
#endif

static uint8_t s_cs_pin = 0;
static uint32_t s_last_sent_id = 0;

void can_driver_init(uint8_t cs_pin) {
    s_cs_pin = cs_pin;
#ifdef ARDUINO
    pinMode(s_cs_pin, OUTPUT);
    digitalWrite(s_cs_pin, HIGH);
    SPI.begin();
#endif
}

bool can_driver_send_frame(uint32_t id, uint8_t* data, uint8_t len) {
    if (data == NULL || len == 0 || len > 8) {
        return false;
    }

#ifdef ARDUINO
    // Mock SPI transmission to MCP2515
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    digitalWrite(s_cs_pin, LOW);
    
    // Command LOAD_TX_BUFFER, etc. (Mock implementation)
    SPI.transfer(0x40); // 0x40 = Load TX Buffer 0
    SPI.transfer((id >> 24) & 0xFF);
    SPI.transfer((id >> 16) & 0xFF);
    SPI.transfer((id >> 8) & 0xFF);
    SPI.transfer(id & 0xFF);
    SPI.transfer(len);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    
    digitalWrite(s_cs_pin, HIGH);
    SPI.endTransaction();
#endif

    s_last_sent_id = id;
    return true;
}

uint32_t can_driver_get_last_id(void) {
    return s_last_sent_id;
}
