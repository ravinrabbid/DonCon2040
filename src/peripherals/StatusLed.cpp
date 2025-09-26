#include "peripherals/StatusLed.h"

#include "hardware/gpio.h"
#include "pio_ws2812/ws2812.h"

namespace Doncon::Peripherals {

StatusLed::StatusLed(const Config &config) : m_config(config), m_input_state({}), m_player_color(std::nullopt) {
    gpio_init(m_config.led_enable_pin);
    gpio_set_dir(m_config.led_enable_pin, GPIO_OUT);
    gpio_put(m_config.led_enable_pin, true);

    ws2812_init(config.led_pin, m_config.is_rgbw);
}

void StatusLed::setBrightness(const uint8_t brightness) { m_config.brightness = brightness; }
void StatusLed::setEnablePlayerColor(const bool do_enable) { m_config.enable_player_color = do_enable; }

void StatusLed::setInputState(const Utils::InputState &input_state) { m_input_state = input_state; }
void StatusLed::setPlayerColor(const Config::Color &color) { m_player_color = color; }

void StatusLed::update() {
    const float brightness_factor = (float)m_config.brightness / (float)UINT8_MAX;

    Config::Color mixed = {};
    bool triggered = false;

    const auto add_color = [](Config::Color &base, const Config::Color &add) {
        base.r = std::max(base.r, add.r);
        base.g = std::max(base.g, add.g);
        base.b = std::max(base.b, add.b);
    };

    if (m_input_state.drum.don_left.triggered) {
        add_color(mixed, m_config.don_left_color);
        triggered = true;
    }
    if (m_input_state.drum.ka_left.triggered) {
        add_color(mixed, m_config.ka_left_color);
        triggered = true;
    }
    if (m_input_state.drum.don_right.triggered) {
        add_color(mixed, m_config.don_right_color);
        triggered = true;
    }
    if (m_input_state.drum.ka_right.triggered) {
        add_color(mixed, m_config.ka_right_color);
        triggered = true;
    }

    if (triggered) {
        ws2812_put_pixel(
            ws2812_rgb_to_gamma_corrected_u32pixel(static_cast<uint8_t>((float)mixed.r * brightness_factor),
                                                   static_cast<uint8_t>((float)mixed.g * brightness_factor),
                                                   static_cast<uint8_t>((float)mixed.b * brightness_factor)));
    } else {
        const auto idle_color =
            m_config.enable_player_color ? m_player_color.value_or(m_config.idle_color) : m_config.idle_color;

        ws2812_put_pixel(
            ws2812_rgb_to_gamma_corrected_u32pixel(static_cast<uint8_t>((float)idle_color.r * brightness_factor),
                                                   static_cast<uint8_t>((float)idle_color.g * brightness_factor),
                                                   static_cast<uint8_t>((float)idle_color.b * brightness_factor)));
    }
}

} // namespace Doncon::Peripherals