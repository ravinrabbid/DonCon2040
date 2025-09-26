#ifndef PERIPHERALS_CONTROLLER_H_
#define PERIPHERALS_CONTROLLER_H_

#include "utils/InputState.h"

#include "hardware/i2c.h"
#include <mcp23017/Mcp23017.h>

#include <cstdint>
#include <map>
#include <memory>
#include <variant>

namespace Doncon::Peripherals {

class Controller {
  public:
    struct Config {
        struct Pins {
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
        };

        struct InternalGpio {};

        struct ExternalGpio {
            struct {
                i2c_inst_t *block;
                uint8_t address;
            } i2c;
        };

        Pins pins;
        uint8_t debounce_delay_ms;

        std::variant<InternalGpio, ExternalGpio> gpio_config;
    };

  private:
    enum class Id : uint8_t {
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
        uint8_t m_gpio_pin;
        uint32_t m_gpio_mask;

        uint32_t m_last_change{0};
        bool m_active{false};

      public:
        Button(uint8_t pin);

        [[nodiscard]] uint8_t getGpioPin() const { return m_gpio_pin; };
        [[nodiscard]] uint32_t getGpioMask() const { return m_gpio_mask; };

        [[nodiscard]] bool getState() const { return m_active; };
        void setState(bool state, uint8_t debounce_delay);
    };

    struct SocdState {
        Id lastVertical;
        Id lastHorizontal;
    };

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Class has no members
    class GpioInterface {
      public:
        virtual ~GpioInterface() = default;
        virtual uint32_t read() = 0;
    };

    class InternalGpio : public GpioInterface {
      public:
        InternalGpio(const std::map<Id, Button> &buttons);
        uint32_t read() final;
    };

    class ExternalGpio : public GpioInterface {
      private:
        Mcp23017 m_mcp23017;

      public:
        ExternalGpio(const Config::ExternalGpio &config);
        uint32_t read() final;
    };

    Config m_config;
    SocdState m_socd_state;
    std::map<Id, Button> m_buttons;

    std::unique_ptr<GpioInterface> m_gpio;

    void socdClean(Utils::InputState &input_state);

  public:
    Controller(const Config &config);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Doncon::Peripherals

#endif // PERIPHERALS_CONTROLLER_H_