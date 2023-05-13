#ifndef _GLOBALCONFIGURATION_H_
#define _GLOBALCONFIGURATION_H_

#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"

namespace Doncon::Config::Default {

const Peripherals::Drum::Config drum_config = {
    // Pin config
    {
        0, // Don Left
        1, // Ka Left
        2, // Don Right
        3, // Ka Right
    },
    400, // Trigger threshold
    100,  // Double hit threshold
    17,  // Debounce delay in milliseconds
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