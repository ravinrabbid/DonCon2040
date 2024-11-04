#ifndef _PERIPHERALS_STATUSLED_H_
#define _PERIPHERALS_STATUSLED_H_

#include "utils/InputState.h"

#include <optional>
#include <stdint.h>

namespace Doncon::Peripherals {

class StatusLed {
  public:
    struct Config {
        struct Color {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        Color idle_color;
        Color don_left_color;
        Color ka_left_color;
        Color don_right_color;
        Color ka_right_color;

        uint8_t led_enable_pin;
        uint8_t led_pin;
        bool is_rgbw;

        uint8_t brightness;
        bool enable_player_color;
    };

  private:
    Config m_config;

    Utils::InputState m_input_state;
    std::optional<Config::Color> m_player_color;

  public:
    StatusLed(const Config &config);

    void setBrightness(const uint8_t brightness);
    void setEnablePlayerColor(const bool do_enable);

    void setInputState(const Utils::InputState input_state);
    void setPlayerColor(const Config::Color color);

    void update();
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_STATUSLED_H_