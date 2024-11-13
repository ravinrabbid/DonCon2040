#include "Mcp3204Dma.h"

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "pico/time.h"

namespace {
constexpr size_t transfer_length = 3;

volatile uint cs_pin = 0;

volatile int rx_channel = -1;
volatile int tx_channel = -1;

volatile uint8_t rx_buffer[transfer_length] = {};
volatile uint8_t tx_buffer[transfer_length] = {
    0x06, // '00000' to align the ADC's output,
          // '1' as start bit,
          // '1' for single-ended read,
          // '0' for D2 (which is 'don't care' on MCP3204)
    0x00, // Channel bits D1 and D0, followed by '0's as clocks to receive result
    0x00, // Further '0's to receive result
};

volatile uint8_t current_channel = 0;
volatile uint16_t current_max_readings[Mcp3204Dma::channel_count] = {};

// Alarm handler to instantly (re)start DMA reading of the next channel.
int64_t start_dma_read(alarm_id_t id, void *user_data) {
    (void)user_data;
    (void)id;

    // Reset addresses
    dma_channel_set_read_addr(tx_channel, tx_buffer, false);
    dma_channel_set_write_addr(rx_channel, rx_buffer, false);

    // Pull down CS pin and start both TX and RX at the same time
    gpio_put(cs_pin, false);
    dma_start_channel_mask((1 << tx_channel) | (1 << rx_channel));

    // Do not reschedule alarm
    return 0;
}

// Pull up CS pin and start DMA reading after a 1us delay, this is because MCP3204
// needs to be the CS pin high for at least 500ns.
void trigger_dma_read() {
    gpio_put(cs_pin, true);

    add_alarm_in_us(2, start_dma_read, nullptr, true);
}

void read_handler() {
    // The 12 result bits are at the end of the ADC's output.
    const uint16_t value = (static_cast<uint16_t>(rx_buffer[1] & 0x0F) << 8) | rx_buffer[2];

    // We only care for the maximum value since the last read
    if (value > current_max_readings[current_channel]) {
        current_max_readings[current_channel] = value;
    }

    // Advance to the next channel
    current_channel = (current_channel + 1) % Mcp3204Dma::channel_count;
    tx_buffer[1] = static_cast<uint8_t>(current_channel << 6);

    dma_channel_acknowledge_irq0(rx_channel);

    trigger_dma_read();
}

} // namespace

Mcp3204Dma::Mcp3204Dma(spi_inst *spi, uint8_t cs_pin) {

    ::cs_pin = cs_pin;
    ::tx_channel = dma_claim_unused_channel(true);
    ::rx_channel = dma_claim_unused_channel(true);

    // Configure TX Channel
    dma_channel_config tx_channel_config = dma_channel_get_default_config(tx_channel);
    channel_config_set_transfer_data_size(&tx_channel_config, DMA_SIZE_8);
    channel_config_set_dreq(&tx_channel_config, spi_get_dreq(spi, true));
    channel_config_set_read_increment(&tx_channel_config, true);
    channel_config_set_write_increment(&tx_channel_config, false);
    dma_channel_configure(tx_channel, &tx_channel_config, &spi_get_hw(spi)->dr, tx_buffer, transfer_length, false);

    // Configure RX Channel
    dma_channel_config rx_channel_config = dma_channel_get_default_config(rx_channel);
    channel_config_set_transfer_data_size(&rx_channel_config, DMA_SIZE_8);
    channel_config_set_dreq(&rx_channel_config, spi_get_dreq(spi, false));
    channel_config_set_read_increment(&rx_channel_config, false);
    channel_config_set_write_increment(&rx_channel_config, true);
    dma_channel_configure(rx_channel, &rx_channel_config, rx_buffer, &spi_get_hw(spi)->dr, transfer_length, false);

    irq_set_exclusive_handler(DMA_IRQ_0, read_handler);
}

Mcp3204Dma::~Mcp3204Dma() {
    stop();

    dma_channel_unclaim(rx_channel);
    dma_channel_unclaim(tx_channel);
}

void Mcp3204Dma::run() {
    stop();

    dma_channel_set_irq0_enabled(rx_channel, true);
    irq_set_enabled(DMA_IRQ_0, true);

    trigger_dma_read();
}

void Mcp3204Dma::stop() {
    irq_set_enabled(DMA_IRQ_0, false);
    dma_channel_set_irq0_enabled(rx_channel, false);

    dma_channel_wait_for_finish_blocking(rx_channel);
    dma_channel_wait_for_finish_blocking(tx_channel);
}

std::array<uint16_t, Mcp3204Dma::channel_count> Mcp3204Dma::take_maximums() {
    // TODO: theoretically we should need to pause conversion for reading the values,
    //       but so far this does not seem to pose any issue.

    std::array<uint16_t, Mcp3204Dma::channel_count> result;
    std::copy(std::begin(current_max_readings), std::end(current_max_readings), std::begin(result));

    // Reset values to zero
    std::fill(std::begin(current_max_readings), std::end(current_max_readings), 0);

    return result;
}