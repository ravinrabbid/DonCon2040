#include "Mcp3204.h"

Mcp3204::Mcp3204(spi_inst *spi) : m_spi(spi) {
    // Put SPI in 1,1 mode. In this mode date will be clocked out on
    // a falling edge and latched from the ADC on a rising edge.
    // Also the CS will be held low in-between bytes as required
    // by the mcp3204.
    spi_set_format(m_spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

uint16_t Mcp3204::read(uint8_t channel) {
    // Byte 1: '00000' to align the ADC's output,
    //         '1' as start bit,
    //         '1' for single-ended read,
    //         '0' for D2 (which is 'don't care' on MCP3204)
    // Byte 2: Channel bits D1 and D0, followed by '0's as clocks to receive result
    // Byte 3: Further '0's to receive result
    uint8_t data_out[3] = {0x06, static_cast<uint8_t>(channel << 6), 0x00};
    uint8_t data_in[3] = {};

    spi_write_read_blocking(m_spi, data_out, data_in, 3);

    // The 12 result bits are at the end of the ADC's output.
    return (static_cast<uint16_t>(data_in[1] & 0x0F) << 8) | data_in[2];
}