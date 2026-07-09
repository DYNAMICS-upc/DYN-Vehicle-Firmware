#include "ads8688_driver.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

#define ADC_VMAX       5.12f
#define PULLUP_V       5.0f
#define PULLUP_OHM     10000.0f
#define CMD_NOP        0x0000
#define CMD_RST_ADC    0x8500

static const uint16_t CMD_MAN[8] = {0xC000, 0xC400, 0xC800, 0xCC00, 0xD000, 0xD400, 0xD800, 0xDC00};

#define BOSCH_N 18
static const float BOSCH_T[18] = {-40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130};
static const float BOSCH_R[18] = {45313, 26114, 15462, 9397, 5896, 3792, 2500, 1707, 1175, 834, 596, 436, 323, 243, 187, 144, 113, 89};

#if defined(ESP_PLATFORM)
#include "driver/spi_master.h"
#include "driver/gpio.h"
static spi_device_handle_t s_spi_handle = NULL;
#endif

float ads8688_driver_bosch_r2t(float r) {
    if (isnan(r) || r <= 0.0f) return NAN;
    if (r > BOSCH_R[0] || r < BOSCH_R[BOSCH_N - 1]) return NAN;
    float lr = logf(r);
    for (int i = 0; i < BOSCH_N - 1; i++) {
        if (r <= BOSCH_R[i] && r >= BOSCH_R[i + 1]) {
            float ratio = (lr - logf(BOSCH_R[i])) / (logf(BOSCH_R[i + 1]) - logf(BOSCH_R[i]));
            return BOSCH_T[i] + ratio * (BOSCH_T[i + 1] - BOSCH_T[i]);
        }
    }
    return NAN;
}

void ads8688_driver_init(void) {
#if defined(ESP_PLATFORM)
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.mosi_io_num = ADC_MOSI_PIN;
    buscfg.miso_io_num = ADC_MISO_PIN;
    buscfg.sclk_io_num = ADC_SCLK_PIN;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 32;
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.mode = 0;
    devcfg.clock_speed_hz = 5000000; // 5 MHz
    devcfg.spics_io_num = ADC_CS_PIN;
    devcfg.queue_size = 3;
    spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi_handle);

    gpio_reset_pin((gpio_num_t)ADC_RST_PIN);
    gpio_set_direction((gpio_num_t)ADC_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)ADC_RST_PIN, 1);
#endif
}

bool ads8688_driver_read_raw(uint8_t channel, uint16_t* out_raw) {
    if (!out_raw || channel > 7) return false;

#if defined(ESP_PLATFORM)
    if (!s_spi_handle) return false;
    uint16_t cmd = CMD_MAN[channel];
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 16,
        .tx_data = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) }
    };
    spi_device_polling_transmit(s_spi_handle, &t);

    spi_transaction_t t_nop = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 16,
        .tx_data = { 0, 0 }
    };
    spi_device_polling_transmit(s_spi_handle, &t_nop);
    *out_raw = ((uint16_t)t_nop.rx_data[0] << 8) | t_nop.rx_data[1];
    return true;
#else
    *out_raw = 0;
    return true;
#endif
}

bool ads8688_driver_read_temp(uint8_t channel, double* out_temp_c) {
    if (!out_temp_c) return false;

    uint16_t raw = 0;
    if (!ads8688_driver_read_raw(channel, &raw)) return false;

    float v = (float)raw / 65535.0f * ADC_VMAX;
    if (v <= 0.01f || v >= PULLUP_V - 0.01f) return false;

    float r = PULLUP_OHM * v / (PULLUP_V - v);
    float t = ads8688_driver_bosch_r2t(r);

    if (isnan(t) || t < -40.0f || t > 130.0f) return false;

    *out_temp_c = (double)t;
    return true;
}
