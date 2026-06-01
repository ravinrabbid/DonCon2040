#ifndef PIO_PSX_PSX_SPI_H_
#define PIO_PSX_PSX_SPI_H_

#include "hardware/pio.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*psx_spi_data_handler_t)(uint8_t data_in, const volatile uint8_t **data_out, void *user_data);
typedef void (*psx_spi_reset_handler_t)(void *user_data);

void psx_spi_init(PIO pio, uint base_pin, psx_spi_data_handler_t data_handler, psx_spi_reset_handler_t reset_handler,
                  void *user_data);

uint8_t psx_spi_read();
void psx_spi_write(uint8_t write_data);

#ifdef __cplusplus
}
#endif

#endif // PIO_PSX_PSX_SPI_H_