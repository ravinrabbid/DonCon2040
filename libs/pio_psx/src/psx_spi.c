#include "psx_spi.h"
#include "psx_spi.pio.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"

static PIO g_pio = NULL;
static uint g_attention_pin = 0;
static psx_spi_data_handler_t g_data_handler = NULL;
static psx_spi_reset_handler_t g_reset_handler = NULL;
static void *g_user_data = NULL;

static uint g_read_sm = 0;
static uint g_write_sm = 0;
static uint g_read_sm_offset = 0;
static uint g_write_sm_offset = 0;

static int g_pio_irq_num = 0;

void __time_critical_func(attention_callback)(uint gpio, uint32_t event) {
    if (gpio != g_attention_pin || !(event & GPIO_IRQ_EDGE_RISE)) {
        return;
    }

    pio_set_sm_mask_enabled(g_pio, 1 << g_read_sm | 1 << g_write_sm, false);
    irq_set_enabled(g_pio_irq_num, false);

    pio_sm_clear_fifos(g_pio, g_read_sm);
    pio_sm_drain_tx_fifo(g_pio, g_write_sm);
    pio_restart_sm_mask(g_pio, 1 << g_read_sm | 1 << g_write_sm);

    pio_sm_exec(g_pio, g_read_sm, pio_encode_jmp(g_read_sm_offset));
    pio_sm_exec(g_pio, g_write_sm, pio_encode_jmp(g_write_sm_offset));

    if (g_reset_handler) {
        g_reset_handler(g_user_data);
    }

    irq_set_enabled(g_pio_irq_num, true);
    pio_enable_sm_mask_in_sync(g_pio, 1 << g_read_sm | 1 << g_write_sm);
}

void __time_critical_func(pio_irq_handler)() {
    while (!pio_sm_is_rx_fifo_empty(g_pio, g_read_sm)) {
        uint8_t read_data = psx_spi_read();

        if (g_data_handler) {
            const volatile uint8_t *write_data = NULL;
            g_data_handler(read_data, &write_data, g_user_data);

            if (write_data) {
                psx_spi_write(*write_data);
            }
        }
    }
}

void psx_spi_init(PIO pio, uint base_pin, psx_spi_data_handler_t data_handler, psx_spi_reset_handler_t reset_handler,
                  void *user_data) {
    g_pio = pio;
    g_attention_pin = base_pin + OFFSET_ATT;
    g_data_handler = data_handler;
    g_reset_handler = reset_handler;
    g_user_data = user_data;

    gpio_disable_pulls(base_pin + OFFSET_DAT);
    gpio_disable_pulls(base_pin + OFFSET_CMD);
    gpio_disable_pulls(base_pin + OFFSET_ATT);
    gpio_disable_pulls(base_pin + OFFSET_CLK);
    gpio_disable_pulls(base_pin + OFFSET_ACK);

    g_read_sm = pio_claim_unused_sm(pio, true);
    g_write_sm = pio_claim_unused_sm(pio, true);

    g_read_sm_offset = pio_add_program(pio, &psx_spi_read_program);
    g_write_sm_offset = pio_add_program(pio, &psx_spi_write_program);

    psx_spi_read_program_init(pio, g_read_sm, g_read_sm_offset, base_pin);
    psx_spi_write_program_init(pio, g_write_sm, g_write_sm_offset, base_pin);

    gpio_set_irq_enabled_with_callback(base_pin + OFFSET_ATT, GPIO_IRQ_EDGE_RISE, true, attention_callback);

    g_pio_irq_num = pio_get_irq_num(pio, 0);
    pio_set_irqn_source_enabled(pio, 0, pio_get_rx_fifo_not_empty_interrupt_source(g_read_sm), true);
    irq_set_exclusive_handler(g_pio_irq_num, pio_irq_handler);
}

uint8_t psx_spi_read() { return (uint8_t)(pio_sm_get_blocking(g_pio, g_read_sm) >> 24); }

void psx_spi_write(uint8_t write_data) { pio_sm_put_blocking(g_pio, g_write_sm, ~write_data); }