#ifndef _GLOBALCONFIGURATION_H_
#define _GLOBALCONFIGURATION_H_

#include "peripherals/Controller.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"

#include "hardware/i2c.h"

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
    100, // Double hit threshold
    17,  // Debounce delay in milliseconds
};

const Peripherals::Buttons::Config button_config = {
    // I2C config
    {
        6,       // SDA Pin
        7,       // SCL Pin
        i2c1,    // Block
        1000000, // Speed
        0x20,    // Address
    },

    // Pins
    {{
         0, // Up
         1, // Down
         2, // Left
         3, // Right
     },
     {
         8,  // North
         9,  // East
         10, // South
         11, // West

         4,  // L
         12, // R

         13, // Start
         5,  // Select
         14, // Home
         6,  // Share
     }},

    20, // Debounce delay in milliseconds
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