#include "Mcp3204Dma.h"

#include "hardware/dma.h"
#include "hardware/gpio.h"

#include <algorithm>

int Mcp3204Dma::m_rx_channel = -1;
int Mcp3204Dma::m_tx_channel = -1;

std::array<uint8_t, Mcp3204Dma::TRANSFER_LENGTH> Mcp3204Dma::m_rx_buffer = {};
std::array<uint8_t, Mcp3204Dma::TRANSFER_LENGTH> Mcp3204Dma::m_tx_buffer = {
    0x06, // '00000' to align the ADC's output,
          // '1' as start bit,
          // '1' for single-ended read,
          // '0' for D2 (which is 'don't care' on MCP3204)
    0x00, // Channel bits D1 and D0, followed by '0's as clocks to receive result
    0x00, // Further '0's to receive result
};

uint8_t Mcp3204Dma::m_current_channel = 0;
std::array<uint16_t, Mcp3204Dma::CHANNEL_COUNT> Mcp3204Dma::m_current_max_readings = {};

uint8_t Mcp3204Dma::m_cs_pin = UINT8_MAX;

bool Mcp3204Dma::m_is_running = false;

// Alarm handler to instantly (re)start DMA reading of the next channel.
int64_t Mcp3204Dma::alarmHandler(alarm_id_t id, void *user_data) {
    (void)user_data;
    (void)id;

    // Reset addresses
    dma_channel_set_read_addr(m_tx_channel, m_tx_buffer.data(), false);
    dma_channel_set_write_addr(m_rx_channel, m_rx_buffer.data(), false);

    // Pull down CS pin and start both TX and RX at the same time
    gpio_put(m_cs_pin, false);
    dma_start_channel_mask((1 << m_tx_channel) | (1 << m_rx_channel));

    // Do not reschedule alarm
    return 0;
}

// Pull up CS pin and start DMA reading after a 1us delay, this is because MCP3204
// needs to be the CS pin high for at least 500ns.
void Mcp3204Dma::triggerDmaRead() {
    gpio_put(m_cs_pin, true);

    add_alarm_in_us(2, alarmHandler, nullptr, true);
}

void Mcp3204Dma::dmaReadHandler() {
    // The 12 result bits are at the end of the ADC's output.
    const uint16_t value = (static_cast<uint16_t>(m_rx_buffer[1] & 0x0F) << 8) | m_rx_buffer[2];

    // We only care for the maximum value since the last read
    m_current_max_readings.at(m_current_channel) = std::max(m_current_max_readings.at(m_current_channel), value);

    // Advance to the next channel
    m_current_channel = (m_current_channel + 1) % CHANNEL_COUNT;
    m_tx_buffer[1] = static_cast<uint8_t>(m_current_channel << 6);

    dma_channel_acknowledge_irq0(m_rx_channel);

    triggerDmaRead();
}

void Mcp3204Dma::initialize(spi_inst *spi, uint8_t cs_pin) {
    Mcp3204Dma::m_cs_pin = cs_pin;
    Mcp3204Dma::m_tx_channel = dma_claim_unused_channel(true);
    Mcp3204Dma::m_rx_channel = dma_claim_unused_channel(true);

    // Configure TX Channel
    dma_channel_config tx_channel_config = dma_channel_get_default_config(m_tx_channel);
    channel_config_set_transfer_data_size(&tx_channel_config, DMA_SIZE_8);
    channel_config_set_dreq(&tx_channel_config, spi_get_dreq(spi, true));
    channel_config_set_read_increment(&tx_channel_config, true);
    channel_config_set_write_increment(&tx_channel_config, false);
    dma_channel_configure(m_tx_channel, &tx_channel_config, &spi_get_hw(spi)->dr, m_tx_buffer.data(), TRANSFER_LENGTH,
                          false);

    // Configure RX Channel
    dma_channel_config rx_channel_config = dma_channel_get_default_config(m_rx_channel);
    channel_config_set_transfer_data_size(&rx_channel_config, DMA_SIZE_8);
    channel_config_set_dreq(&rx_channel_config, spi_get_dreq(spi, false));
    channel_config_set_read_increment(&rx_channel_config, false);
    channel_config_set_write_increment(&rx_channel_config, true);
    dma_channel_configure(m_rx_channel, &rx_channel_config, m_rx_buffer.data(), &spi_get_hw(spi)->dr, TRANSFER_LENGTH,
                          false);

    irq_set_exclusive_handler(DMA_IRQ_0, dmaReadHandler);
}

void Mcp3204Dma::run(spi_inst *spi, uint8_t cs_pin) {
    if (m_is_running) {
        stop();
    }

    initialize(spi, cs_pin);

    m_is_running = true;

    dma_channel_set_irq0_enabled(m_rx_channel, true);
    irq_set_enabled(DMA_IRQ_0, true);

    triggerDmaRead();
}

void Mcp3204Dma::stop() {
    if (!m_is_running) {
        return;
    }

    irq_set_enabled(DMA_IRQ_0, false);
    dma_channel_set_irq0_enabled(m_rx_channel, false);

    dma_channel_wait_for_finish_blocking(m_rx_channel);
    dma_channel_wait_for_finish_blocking(m_tx_channel);

    dma_channel_unclaim(m_rx_channel);
    dma_channel_unclaim(m_tx_channel);
}

std::array<uint16_t, Mcp3204Dma::CHANNEL_COUNT> Mcp3204Dma::take_maximums() {
    // TODO: theoretically we should need to pause conversion for reading the values,
    //       but so far this does not seem to pose any issue.
    std::array<uint16_t, Mcp3204Dma::CHANNEL_COUNT> result{m_current_max_readings};

    // Reset values to zero
    std::ranges::fill(m_current_max_readings, 0);

    return result;
}