#ifndef _PERIPHERALS_DRUM_H_
#define _PERIPHERALS_DRUM_H_

#include "utils/InputState.h"

#include <map>
#include <stdint.h>

namespace Doncon::Peripherals {

class Drum {
  public:
    struct Config {
        struct {
            uint8_t don_left_weak;
            uint8_t ka_left_weak;
            uint8_t don_right_weak;
            uint8_t ka_right_weak;

            uint8_t don_left_strong;
            uint8_t ka_left_strong;
            uint8_t don_right_strong;
            uint8_t ka_right_strong;
        } pins;

        uint8_t debounce_delay_ms;
    };

  private:
    enum class Id {
        DON_LEFT_WEAK,
        KA_LEFT_WEAK,
        DON_RIGHT_WEAK,
        KA_RIGHT_WEAK,
        DON_LEFT_STRONG,
        KA_LEFT_STRONG,
        DON_RIGHT_STRONG,
        KA_RIGHT_STRONG,
    };

    class Pad {
      private:
        uint8_t gpio_pin;
        uint32_t gpio_mask;

        uint32_t last_change;
        bool active;

      public:
        Pad(uint8_t pin);

        uint8_t getGpioPin() const { return gpio_pin; };
        uint32_t getGpioMask() const { return gpio_mask; };

        bool getState() const { return active; };
        void setState(bool state, uint8_t debounce_delay);
    };

    Config m_config;
    std::map<Id, Pad> m_pads;

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DRUM_H_