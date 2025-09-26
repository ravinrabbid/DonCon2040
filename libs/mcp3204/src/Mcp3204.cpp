#include "Mcp3204.h"

#include "hardware/gpio.h"

#include <array>

Mcp3204::Mcp3204(spi_inst *spi, uint8_t cs_pin) : m_spi(spi), m_cs_pin(cs_pin) {}

uint16_t Mcp3204::read(uint8_t channel) {
    // Byte 1: '00000' to align the ADC's output,
    //         '1' as start bit,
    //         '1' for single-ended read,
    //         '0' for D2 (which is 'don't care' on MCP3204)
    // Byte 2: Channel bits D1 and D0, followed by '0's as clocks to receive result
    // Byte 3: Further '0's to receive result
    std::array<uint8_t, 3> data_out = {0x06, static_cast<uint8_t>(channel << 6), 0x00};
    std::array<uint8_t, 3> data_in = {};

    gpio_put(m_cs_pin, false);
    spi_write_read_blocking(m_spi, data_out.data(), data_in.data(), 3);
    gpio_put(m_cs_pin, true);

    // The 12 result bits are at the end of the ADC's output.
    return (static_cast<uint16_t>(data_in[1] & 0x0F) << 8) | data_in[2];
}