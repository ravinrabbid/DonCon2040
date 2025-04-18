#ifndef _MCP3204_MCP3204DMA_H_
#define _MCP3204_MCP3204DMA_H_

#include "hardware/spi.h"

#include <array>

class Mcp3204Dma {
  public:
    static constexpr size_t channel_count = 4;

  public:
    Mcp3204Dma(spi_inst *spi, uint8_t cs_pin);
    ~Mcp3204Dma();

    void run();
    void stop();

    std::array<uint16_t, channel_count> take_maximums();
};

#endif // _MCP3204_MCP3204DMA_H_