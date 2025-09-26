#ifndef MCP3204_MCP3204DMA_H_
#define MCP3204_MCP3204DMA_H_

#include "hardware/spi.h"
#include "pico/time.h"

#include <array>

class Mcp3204Dma {
  private:
    static constexpr size_t CHANNEL_COUNT = 4;
    static constexpr size_t TRANSFER_LENGTH = 3;

    static int m_rx_channel;
    static int m_tx_channel;

    static std::array<uint8_t, TRANSFER_LENGTH> m_rx_buffer;
    static std::array<uint8_t, TRANSFER_LENGTH> m_tx_buffer;

    static uint8_t m_current_channel;
    static std::array<uint16_t, CHANNEL_COUNT> m_current_max_readings;

    static uint8_t m_cs_pin;

    static bool m_is_running;

    static int64_t alarmHandler(alarm_id_t id, void *user_data);
    static void triggerDmaRead();
    static void dmaReadHandler();

    static void initialize(spi_inst *spi, uint8_t cs_pin);

  public:
    Mcp3204Dma() = delete;

    static void run(spi_inst *spi, uint8_t cs_pin);
    static void stop();

    static std::array<uint16_t, CHANNEL_COUNT> take_maximums();
};

#endif // MCP3204_MCP3204DMA_H_