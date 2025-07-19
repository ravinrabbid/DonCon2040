#ifndef _GLOBALCONFIGURATION_H_
#define _GLOBALCONFIGURATION_H_

#include "peripherals/Controller.h"
#include "peripherals/Display.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"

#include "hardware/i2c.h"
#include "hardware/spi.h"

namespace Doncon::Config {

struct I2c {
    uint8_t sda_pin;
    uint8_t scl_pin;
    i2c_inst_t *block;
    uint speed_hz;
};

namespace Default {

const usb_mode_t usb_mode = USB_MODE_SWITCH_TATACON;

const I2c i2c_config = {
    14,      // SDA Pin
    15,      // SCL Pin
    i2c1,    // Block
    1000000, // Speed
};

const Peripherals::Drum::Config drum_config = {
    // Trigger thresholds
    {
        10, // Don Left
        5,  // Ka Left
        10, // Don Right
        5,  // Ka Right
    },
    25,  // Debounce delay in milliseconds
    500, // Roll Counter Timeout in Milliseconds

    // ADC Channel config
    {
        3, // Don Left
        2, // Ka Left
        0, // Don Right
        1, // Ka Right
    },

    // ADC Config, either InternalAdc or ExternalAdc
    //
    // Peripherals::Drum::Config::InternalAdc{
    //     16, // ADC sample count
    // },

    Peripherals::Drum::Config::ExternalAdc{
        spi1,    // Block
        2000000, // Speed
        11,      // MOSI Pin
        12,      // MISO Pin
        10,      // SCLK Pin
        13,      // SCSn Pin
        9,       // Level Shifter Enable Pin
    },
};

const Peripherals::Controller::Config controller_config = {

    // Pins
    {{
         8,  // Up
         9,  // Down
         10, // Left
         11, // Right
     },
     {
         0, // North
         3, // East
         1, // South
         2, // West

         12, // L
         4,  // R

         5,  // Start
         13, // Select
         6,  // Home
         14, // Share
     }},

    25, // Debounce delay in milliseconds

    // GPIO Config, either InternalGpio or ExternalGpio
    //
    // Peripherals::Controller::Config::InternalGpio{},

    Peripherals::Controller::Config::ExternalGpio{
        {
            i2c_config.block, // Block
            0x20,             // Address
        },
    },
};

const Peripherals::StatusLed::Config led_config = {
    {128, 128, 128}, // Idle Color
    {255, 0, 0},     // Don Left Color
    {0, 0, 255},     // Ka Left Color
    {255, 255, 0},   // Don Right Color
    {0, 255, 255},   // Ka Right Color

    25,    // LED Enable Pin,
    16,    // LED Pin
    false, // Is RGBW

    255,  // Brightness
    true, // Idle Color is DS4 light bar color
};

const Peripherals::Display::Config display_config = {
    i2c_config.block, // Block
    0x3C,             // Address
};

} // namespace Default
} // namespace Doncon::Config

#endif // _GLOBALCONFIGURATION_H_