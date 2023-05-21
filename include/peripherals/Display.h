#ifndef _PERIPHERALS_DISPLAY_H_
#define _PERIPHERALS_DISPLAY_H_

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

    ssd1306_t m_display;

    void drawIdleScreen();
    void drawMenuScreen();

  public:
    Display(const Config &config);

    void showIdle();
    void showMenu();

    void update();
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DISPLAY_H_