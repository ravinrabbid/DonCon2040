#ifndef _PERIPHERALS_BUTTONS_H_
#define _PERIPHERALS_BUTTONS_H_

#include "utils/InputState.h"

#include <mcp23017/Mcp23017.h>

#include <map>
#include <memory>
#include <stdint.h>

namespace Doncon::Peripherals {

class Buttons {
  public:
    struct Config {

        struct {
            uint8_t sda_pin;
            uint8_t scl_pin;
            i2c_inst_t *block;
            uint speed_hz;
            uint8_t address;
        } i2c;

        struct {
            struct {
                uint8_t up;
                uint8_t down;
                uint8_t left;
                uint8_t right;
            } dpad;

            struct {
                uint8_t north;
                uint8_t east;
                uint8_t south;
                uint8_t west;

                uint8_t l;
                uint8_t r;

                uint8_t start;
                uint8_t select;
                uint8_t home;
                uint8_t share;
            } buttons;
        } pins;

        uint8_t debounce_delay_ms;
    };

  private:
    enum class Id {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        NORTH,
        EAST,
        SOUTH,
        WEST,
        L,
        R,
        START,
        SELECT,
        HOME,
        SHARE,
    };

    class Button {
      private:
        uint8_t gpio_pin;
        uint16_t gpio_mask;

        uint32_t last_change;
        bool active;

      public:
        Button(uint8_t pin);

        uint8_t getGpioPin() const { return gpio_pin; };
        uint16_t getGpioMask() const { return gpio_mask; };

        bool getState() const { return active; };
        void setState(bool state, uint8_t debounce_delay);
    };

    struct SocdState {
        Id lastVertical;
        Id lastHorizontal;
    };

    Config m_config;
    SocdState m_socd_state;
    std::map<Id, Button> m_buttons;

    std::unique_ptr<Mcp23017> m_mcp23017;

    void socdClean(Utils::InputState &input_state);

  public:
    Buttons(const Config &config);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_BUTTONS_H_