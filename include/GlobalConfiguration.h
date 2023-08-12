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

const usb_mode_t usb_mode = USB_MODE_XBOX360;

const I2c i2c_config = {
    6,       // SDA Pin
    7,       // SCL Pin
    i2c1,    // Block
    1000000, // Speed
};

const Peripherals::Drum::Config drum_config = {
    // Pin config
    {
        2, // Don Left
        3, // Ka Left
        1, // Don Right
        0, // Ka Right
    },
    // Trigger thresholds soft
    {
        80, // Don Left
        50,  // Ka Left
        80, // Don Right
        50,  // Ka Right
    },
    230, // Trigger threshold scale level

    50, // ADC sample count
    25, // Debounce delay in milliseconds

    true, // Use external ADC
    // SPI config for external ADC, unused if above is false
    {
        3,       // MOSI Pin
        4,       // MISO Pin
        2,       // SCLK Pin
        1,       // SCSn Pin
        spi0,    // Block
        2000000, // Speed
    },
};

const Peripherals::Buttons::Config button_config = {
    // I2c config
    {
        i2c_config.block, // Block
        0x20,             // Address
    },

    // Pins
    {{
         0, // Up
         1, // Down
         2, // Left
         3, // Right
     },
     {
         10, // North
         9,  // East
         8,  // South
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

const Peripherals::Display::Config display_config = {
    i2c_config.block, // Block
    0x3C,             // Address

    500, // Roll Counter Timeout in Milliseconds
};

} // namespace Default
} // namespace Doncon::Config

#endif // _GLOBALCONFIGURATION_H_