#ifndef _GLOBALCONFIGURATION_H_
#define _GLOBALCONFIGURATION_H_

#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"

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

const Peripherals::StatusLed::Config led_config = {
    {255, 0, 0},   // Don Left Color
    {0, 0, 255},   // Ka Left Color
    {255, 255, 0}, // Don Right Color
    {0, 255, 255}, // Ka Right Color

    11,    // LED Enable Pin,
    12,    // LED Pin
    false, // Is RGBW
    255,   // Brightness
};

} // namespace Doncon::Config::Default

#endif // _GLOBALCONFIGURATION_H_