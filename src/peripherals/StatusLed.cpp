#include "peripherals/StatusLed.h"

#include "hardware/gpio.h"
#include "pio_ws2812/ws2812.h"

namespace Doncon::Peripherals {

StatusLed::StatusLed(const Config &config) : m_config(config) {
    gpio_init(m_config.led_enable_pin);
    gpio_set_dir(m_config.led_enable_pin, GPIO_OUT);
    gpio_put(m_config.led_enable_pin, 1);

    ws2812_init(config.led_pin, m_config.is_rgbw);
}

void StatusLed::setInputState(const Utils::InputState input_state) { m_input_state = input_state; }

void StatusLed::update() {
    static float brightness_factor = m_config.brightness / static_cast<float>(UINT8_MAX);

    uint16_t mixed_red = 0;
    uint16_t mixed_green = 0;
    uint16_t mixed_blue = 0;

    uint8_t num_colors = 0;

    if (m_input_state.drum.don_left.triggered) {
        mixed_red += m_config.don_left_color.r;
        mixed_green += m_config.don_left_color.g;
        mixed_blue += m_config.don_left_color.b;
        num_colors++;
    }
    if (m_input_state.drum.ka_left.triggered) {
        mixed_red += m_config.ka_left_color.r;
        mixed_green += m_config.ka_left_color.g;
        mixed_blue += m_config.ka_left_color.b;
        num_colors++;
    }
    if (m_input_state.drum.don_right.triggered) {
        mixed_red += m_config.don_right_color.r;
        mixed_green += m_config.don_right_color.g;
        mixed_blue += m_config.don_right_color.b;
        num_colors++;
    }
    if (m_input_state.drum.ka_right.triggered) {
        mixed_red += m_config.ka_right_color.r;
        mixed_green += m_config.ka_right_color.g;
        mixed_blue += m_config.ka_right_color.b;
        num_colors++;
    }

    if (num_colors > 0) {
        ws2812_put_pixel(ws2812_rgb_to_gamma_corrected_u32pixel(
            static_cast<uint8_t>((mixed_red / num_colors) * brightness_factor),
            static_cast<uint8_t>((mixed_green / num_colors) * brightness_factor),
            static_cast<uint8_t>((mixed_blue / num_colors) * brightness_factor)));
    } else {
        ws2812_put_pixel(0);
    }
}

} // namespace Doncon::Peripherals