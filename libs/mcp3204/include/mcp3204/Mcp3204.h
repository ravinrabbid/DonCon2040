#ifndef MCP3204_MCP3204_H_
#define MCP3204_MCP3204_H_

#include "hardware/spi.h"

class Mcp3204 {
  private:
    spi_inst *m_spi;
    uint8_t m_cs_pin;

  public:
    Mcp3204(spi_inst *spi, uint8_t cs_pin);

    uint16_t read(uint8_t channel);
};

#endif // MCP3204_MCP3204_H_