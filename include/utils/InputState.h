#ifndef _UTILS_INPUTSTATE_H_
#define _UTILS_INPUTSTATE_H_

#include <stdint.h>

namespace Doncon::Utils {

struct InputState {
  public:
    struct Drum {
        bool don_left, ka_left, don_right, ka_right;
    };

  public:
    Drum drum;

  public:
    InputState();
};

} // namespace Doncon::Utils

#endif // _UTILS_INPUTSTATE_H_