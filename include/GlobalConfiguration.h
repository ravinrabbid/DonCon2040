#ifndef GLOBALCONFIGURATION_H_
#define GLOBALCONFIGURATION_H_

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
    .sda_pin = 14,
    .scl_pin = 15,
    .block = i2c1,
    .speed_hz = 1000000,
};

const Peripherals::Drum::Config drum_config = {
    .trigger_thresholds =
        {
            .don_left = 10,
            .ka_left = 5,
            .don_right = 10,
            .ka_right = 5,
        },

    .double_trigger_mode = Peripherals::Drum::Config::DoubleTriggerMode::Off,
    .double_trigger_thresholds =
        {
            .don_left = 2000,
            .ka_left = 1500,
            .don_right = 2000,
            .ka_right = 1500,
        },

    .debounce_delay_ms = 25,
    .roll_counter_timeout_ms = 500,

    .adc_channels =
        {
            .don_left = 3,
            .ka_left = 2,
            .don_right = 0,
            .ka_right = 1,
        },

    // ADC Config, either InternalAdc or ExternalAdc
    // .adc_config =
    //     Peripherals::Drum::Config::InternalAdc{
    //         .sample_count = 16,
    //     },

    .adc_config =
        Peripherals::Drum::Config::ExternalAdc{
            .spi_block = spi1,
            .spi_speed_hz = 2000000,
            .spi_mosi_pin = 11,
            .spi_miso_pin = 12,
            .spi_sclk_pin = 10,
            .spi_scsn_pin = 13,
            .spi_level_shifter_enable_pin = 9,
        },
};

const Peripherals::Controller::Config controller_config = {
    .pins =
        {
            .dpad =
                {
                    .up = 8,
                    .down = 9,
                    .left = 10,
                    .right = 11,
                },
            .buttons =
                {
                    .north = 0,
                    .east = 3,
                    .south = 1,
                    .west = 2,

                    .l = 12,
                    .r = 4,

                    .start = 5,
                    .select = 13,
                    .home = 6,
                    .share = 14,
                },
        },

    .debounce_delay_ms = 25,

    // GPIO Config, either InternalGpio or ExternalGpio
    // .gpio_config = Peripherals::Controller::Config::InternalGpio{},
    .gpio_config =
        Peripherals::Controller::Config::ExternalGpio{
            .i2c =
                {
                    .block = i2c_config.block,
                    .address = 0x20,
                },
        },
};

const Peripherals::StatusLed::Config led_config = {
    .idle_color = {.r = 128, .g = 128, .b = 128},
    .don_left_color = {.r = 255, .g = 0, .b = 0},
    .ka_left_color = {.r = 0, .g = 0, .b = 255},
    .don_right_color = {.r = 255, .g = 255, .b = 0},
    .ka_right_color = {.r = 0, .g = 255, .b = 255},

    .led_enable_pin = 25,
    .led_pin = 16,
    .is_rgbw = false,

    .brightness = 255,
    .enable_player_color = true,
};

const Peripherals::Display::Config display_config = {
    .i2c_block = i2c_config.block,
    .i2c_address = 0x3C,
};

} // namespace Default
} // namespace Doncon::Config

#endif // GLOBALCONFIGURATION_H_