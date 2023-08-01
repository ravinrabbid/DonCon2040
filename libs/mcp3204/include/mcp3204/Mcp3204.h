#ifndef _MCP3204_MCP3204_H_
#define _MCP3204_MCP3204_H_

#include "hardware/spi.h"

class Mcp3204 {
  private:
    spi_inst *m_spi;

  public:
    Mcp3204(spi_inst *spi);

    uint16_t read(uint8_t channel);
};

#endif // _MCP3204_MCP3204_H_