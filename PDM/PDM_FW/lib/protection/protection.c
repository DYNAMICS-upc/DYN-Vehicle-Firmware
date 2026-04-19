#include "protection.h"

void protection_init(void) {
    // Inicialización del módulo de protecciones
}

bool protection_check_mux_channel(uint8_t channel, uint16_t current_val) {
    // Si la lectura supera el límite máximo de ADC para el canal, es fallo (false = no seguro)
    if (current_val > MUX_CHANNEL_MAX_CURRENT) {
        return false;
    }
    return true;
}
