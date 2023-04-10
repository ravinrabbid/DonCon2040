#ifndef _GLOBALCONFIGURATION_H_
#define _GLOBALCONFIGURATION_H_

#include "peripherals/Drum.h"

namespace Doncon::Config::Default {

const Peripherals::Drum::Config drum_config = {
    // Pin config
    {
        4, // Don Left Weak
        3, // Ka Left Weak
        2, // Don Right Weak
        1, // Ka Right Weak

        6,  // Don Left Strong
        29, // Ka Left Strong
        7,  // Don Right Strong
        0,  // Ka Right Strong
    },
    10, // Debounce delay in milliseconds
};

} // namespace Doncon::Config::Default

#endif // _GLOBALCONFIGURATION_H_