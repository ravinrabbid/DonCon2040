#ifndef _PERIPHERALS_DISPLAY_H_
#define _PERIPHERALS_DISPLAY_H_

#include "usb/usb_driver.h"
#include "utils/InputState.h"

#include <ssd1306/ssd1306.h>

#include "hardware/i2c.h"

#include <memory>
#include <stdint.h>

namespace Doncon::Peripherals {

class Display {
  public:
    struct Config {
        i2c_inst_t *i2c_block;
        uint8_t i2c_address;
    };

  private:
    enum class State {
        Idle,
        Menu,
    };

    Config m_config;
    State m_state;

    Utils::InputState m_input_state;
    usb_mode_t m_usb_mode;
    uint8_t m_player_id;

    ssd1306_t m_display;

    void drawIdleScreen();
    void drawMenuScreen();

  public:
    Display(const Config &config);

    void setInputState(const Utils::InputState &state);
    void setUsbMode(usb_mode_t mode);
    void setPlayerId(uint8_t player_id);

    void showIdle();
    void showMenu();

    void update();
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DISPLAY_H_