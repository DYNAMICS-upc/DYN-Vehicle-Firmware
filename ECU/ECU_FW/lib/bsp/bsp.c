#include "bsp.h"
#include "fan_driver.h"
#include "ads8688_driver.h"

void bsp_init(void) {
    fan_driver_init();
    ads8688_driver_init(); // Initialize the ADC driver
}
