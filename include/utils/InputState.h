#ifndef UTILS_INPUTSTATE_H_
#define UTILS_INPUTSTATE_H_

#include <cstdint>

namespace Doncon::Utils {

struct InputState {
  public:
    struct Drum {
        struct Pad {
            bool triggered;
            uint16_t analog;
            uint16_t raw;
        };

        Pad don_left, ka_left, don_right, ka_right;
        uint16_t current_roll;
        uint16_t previous_roll;
    };

    struct Controller {
        struct DPad {
            bool up, down, left, right;
        };

        struct Buttons {
            bool north, east, south, west;
            bool l, r;
            bool start, select, home, share;
        };

        DPad dpad;
        Buttons buttons;
    };

    Drum drum{};
    Controller controller{};

    void releaseAll() {
        drum = {};
        controller = {};
    };
};

} // namespace Doncon::Utils

#endif // UTILS_INPUTSTATE_H_