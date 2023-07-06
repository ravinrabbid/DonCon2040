#ifndef _PERIPHERALS_DRUM_H_
#define _PERIPHERALS_DRUM_H_

#include "utils/InputState.h"

#include <map>
#include <stdint.h>

namespace Doncon::Peripherals {

class Drum {
  public:
    struct Config {
        struct Thresholds {
            uint16_t don_left;
            uint16_t ka_left;
            uint16_t don_right;
            uint16_t ka_right;
        };

        struct {
            uint8_t don_left;
            uint8_t ka_left;
            uint8_t don_right;
            uint8_t ka_right;
        } pins;

        Thresholds trigger_thresholds;
        uint8_t sample_count;
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
    std::map<Id, uint16_t> sampleInputs();

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DRUM_H_