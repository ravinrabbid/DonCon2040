#ifndef PERIPHERALS_DISPLAY_H_
#define PERIPHERALS_DISPLAY_H_

#include "usb/device_driver.h"
#include "utils/InputState.h"
#include "utils/Menu.h"

#include "hardware/i2c.h"
#include <ssd1306/ssd1306.h>

#include <cstdint>
#include <memory>

namespace Doncon::Peripherals {

class Display {
  public:
    struct Config {
        i2c_inst_t *i2c_block;
        uint8_t i2c_address;
    };

  private:
    enum class State : uint8_t {
        Idle,
        Menu,
    };

    Config m_config;
    State m_state{State::Idle};

    Utils::InputState m_input_state;
    usb_mode_t m_usb_mode{USB_MODE_DEBUG};
    uint8_t m_player_id{0};

    Utils::Menu::State m_menu_state{};

    ssd1306_t m_display{};

    void drawIdleScreen();
    void drawMenuScreen();

  public:
    Display(const Config &config);

    void setInputState(const Utils::InputState &state);
    void setUsbMode(usb_mode_t mode);
    void setPlayerId(uint8_t player_id);

    void setMenuState(const Utils::Menu::State &menu_state);

    void showIdle();
    void showMenu();

    void update();
};

} // namespace Doncon::Peripherals

#endif // PERIPHERALS_DISPLAY_H_