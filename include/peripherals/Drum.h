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
            uint8_t don_left;
            uint8_t ka_left;
            uint8_t don_right;
            uint8_t ka_right;
        } pins;

        uint16_t trigger_threshold;
        uint16_t double_hit_threshold;

        uint16_t debounce_delay_ms;
    };

  private:
    enum class Id {
        DON_LEFT,
        KA_LEFT,
        DON_RIGHT,
        KA_RIGHT,
    };

    class Pad {
      private:
        uint8_t pin;
        uint32_t last_change;
        bool active;

      public:
        Pad(uint8_t pin);

        uint8_t getPin() const { return pin; };
        bool getState() const { return active; };
        void setState(bool state, uint16_t debounce_delay);
    };

    Config m_config;
    std::map<Id, Pad> m_pads;

  private:
    std::map<Id, uint16_t> sampleInputs(uint8_t count, uint16_t delay_us);

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DRUM_H_