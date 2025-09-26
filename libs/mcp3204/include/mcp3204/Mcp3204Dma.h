#ifndef MCP3204_MCP3204DMA_H_
#define MCP3204_MCP3204DMA_H_

#include "hardware/spi.h"

#include <array>

class Mcp3204Dma {
  public:
    static constexpr size_t channel_count = 4;

  public:
    Mcp3204Dma(spi_inst *spi, uint8_t cs_pin);
    Mcp3204Dma(const Mcp3204Dma &) = delete;
    Mcp3204Dma(Mcp3204Dma &&) = default;
    Mcp3204Dma &operator=(Mcp3204Dma &&) = delete;
    Mcp3204Dma &operator=(const Mcp3204Dma &) = default;
    ~Mcp3204Dma();

    void run();
    void stop();

    std::array<uint16_t, channel_count> take_maximums();
};

#endif // MCP3204_MCP3204DMA_H_