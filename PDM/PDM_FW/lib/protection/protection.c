#include "protection.h"

// Definir los limites de corriente por canal (valores ADC)
static const uint16_t CHANNEL_LIMITS[8] = {
    800, 500, 1000, 600, 800, 400, 900, 700
};

// Contador de muestras consecutivas por encima del limite
static uint8_t overcurrent_counters[8] = {0};

// Limite de muestras consecutivas para considerar un fallo real (ignorar inrush/picos cortos)
#define MAX_CONSECUTIVE_OVERCURRENT 5

void protection_init(void) {
    for (int i = 0; i < 8; i++) {
        overcurrent_counters[i] = 0;
    }
}

bool protection_check_mux_channel(uint8_t channel, uint16_t current_val) {
    if (channel >= 8) return true;

    if (current_val > CHANNEL_LIMITS[channel]) {
        overcurrent_counters[channel]++;
        if (overcurrent_counters[channel] >= MAX_CONSECUTIVE_OVERCURRENT) {
            // Fusible disparado (Trip)
            return false;
        }
    } else {
        // Reset counter si la corriente baja (enfriamiento)
        overcurrent_counters[channel] = 0;
    }
    
    return true; // Seguro
}
