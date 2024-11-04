#include "peripherals/StatusLed.h"

#include "hardware/gpio.h"
#include "pio_ws2812/ws2812.h"

namespace Doncon::Peripherals {

StatusLed::StatusLed(const Config &config) : m_config(config), m_input_state({}), m_player_color(std::nullopt) {
    gpio_init(m_config.led_enable_pin);
    gpio_set_dir(m_config.led_enable_pin, GPIO_OUT);
    gpio_put(m_config.led_enable_pin, 1);

    ws2812_init(config.led_pin, m_config.is_rgbw);
}

void StatusLed::setBrightness(const uint8_t brightness) { m_config.brightness = brightness; }
void StatusLed::setEnablePlayerColor(const bool do_enable) { m_config.enable_player_color = do_enable; }

void StatusLed::setInputState(const Utils::InputState input_state) { m_input_state = input_state; }
void StatusLed::setPlayerColor(const Config::Color color) { m_player_color = color; }

void StatusLed::update() {
    float brightness_factor = m_config.brightness / static_cast<float>(UINT8_MAX);

    uint16_t mixed_red = 0;
    uint16_t mixed_green = 0;
    uint16_t mixed_blue = 0;

    uint8_t num_colors = 0;

    // TODO simply use max of each channel
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
        const auto idle_color =
            m_config.enable_player_color ? m_player_color.value_or(m_config.idle_color) : m_config.idle_color;

        ws2812_put_pixel(
            ws2812_rgb_to_gamma_corrected_u32pixel(static_cast<uint8_t>((idle_color.r) * brightness_factor),
                                                   static_cast<uint8_t>((idle_color.g) * brightness_factor),
                                                   static_cast<uint8_t>((idle_color.b) * brightness_factor)));
    }
}

} // namespace Doncon::Peripherals