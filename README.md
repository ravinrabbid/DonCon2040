# DonCon2040 - A RP2040 based Taiko no Tatsujin arcade controller

DonCon2040 is a firmware (and by extension a PCB) for DIY Taiko no Tatsujin arcade style drum controllers.

It is pretty much tailored to this specific use case, if you are looking for something universal, ready-to-flash and on-the-fly configurable I'd recommend to have a look at more generic approaches like [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE). If you however want build something more specialized or custom, feel free to use this project as a base, it is designed to be somewhat modular and should be easy remodel. See [DivaCon2040](https://github.com/ravinrabbid/DivaCon2040) for an example on how this could look like.
If you have any questions about the project in general or need hints how to build this thing, feel free to open a [discussion](https://github.com/ravinrabbid/DonCon2040/discussions) anytime!

![DonCon2040](assets/drum.jpg)

## Features

- Various controller emulation modes 
  - HORI PS4-095 Taiko Drum for PS4 (Untested! Will probably timeout after 8 minutes)
  - HORI NSW-079 Taiko Drum for Switch (compatible with Taiko no Tatsujin Rhythm Festival / Drum'n'Fun on Switch)
  - Dualshock 4 (does not work on an actual PS4!)
  - Dualshock 3
  - Switch Pro Controller
  - XInput
  - XInput Analog (Compatible with [TaikoArcadeLoader](https://github.com/esuo1198/TaikoArcadeLoader) analog input)
  - Keyboard (Mapping: 'DFJK' / 'CBN,')
  - MIDI
  - Debug mode (will output current state via USB serial and allow direct flashing)
- Additional buttons via external i2c GPIO expander
- Basic configuration via on-screen menu on attached OLED screen
- Single WS2812 LED for trigger feedback
- Drumroll counter on display

## Building

I highly recommend to build the firmware yourself so you can make adjustments in `include/GlobalConfiguration.h` to match your specific controller build.
You can still use the [binary release](https://github.com/ravinrabbid/DonCon2040/releases) which is pre-configured for the [DonConIO](/pcb/DonConIO).

### VSCode (Windows, Linux, MacOS)

Install [VSCode](https://code.visualstudio.com/) and get the [Raspberry Pi Pico](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) extension. From the extension choose 'Import Project' and select the folder where you've checked out this repository, then use 'Compile Project'.

### CLI

See [pico-sdk readme](https://github.com/raspberrypi/pico-sdk/blob/master/README.md#quick-start-your-own-project) for a list of pre-requisites.

Use the environment variables `PICO_SDK_PATH` to use a local pico-sdk, and `PICO_BOARD` to select another target board.
By default the pico-sdk will be fetched from Github and the target board will be "pico".

```sh
mkdir build && cd build
cmake ..
make
```

## Configuration

Few things which you probably want to change more regularly can be changed using an on-screen menu on the attached OLED display, hold both Start and Select for 2 seconds to enter the menu:

- Controller emulation mode
- LED brightness
- Trigger thresholds
- Hold Time
- Enter BOOTSEL mode for firmware flashing

Those settings are persisted to flash memory if you choose 'Save' when exiting the Menu and will survive power cycles.

Defaults and everything else are compiled statically into the firmware. You can find everything in `include/GlobalConfiguration.h`. This covers default controller emulation mode, i2c pins, external ADC configuration, addresses and speed, default trigger thresholds, scale and debounce delay, button mapping, LED colors and brightness.

### Debounce Delay / Hold Time

The debounce delay also implicitly serves as the hold time of the input after a hit. On some platforms inputs won't be registered properly if this time is too short. For example Taiko no Tatsujin on Switch needs at least 25 milliseconds.

If you notice dropped inputs even if the controller signals a hit on the LED/Display, try to increase this value.

## Hardware

### IO Board

The [DonConIO](/pcb/DonConIO) board in the pcb subfolder is designed to be close to the original arcade hardware. It hosts a Seeed Studio XIAO RP2040 and provides signal conditioning for Sensatec GSS-4S* piezo impact sensors. See its [README](/pcb/DonConIO/README.md) for details.

If you don't want to use this board, the firmware should be usable on most RP2040 boards with appropriate configuration. You may also use a more simple trigger solution, it only has to provide an analog trigger level to the ADC inputs to be compatible.

### Controller Buttons and Display

Additional controller buttons and the display are attached to the same (or different if your board has more than one) i2c bus. For the display, use a standard SSD1306 OLED display with 128x64 resolution. The buttons need to be attached to a MCP23017 IO expander.

See [DonConPad](/pcb/DonConPad/) for a exemplary gamepad pcb.

Mind that currently the display and buttons are mandatory to use the controller.

### Physical construction

I'll only give a rough outline of the physical construction since I'm still not completely happy with its performance and I'm still experimenting with some alternatives:

- The pads are made of 12mm thick multiplex boards. The outer diameter is ~42cm, the inner diameter is ~35cm.
- Pads are mounted to another 20mm multiplex board with rubber dampeners. The backplates on the arcade drum seem to be thinner, but since those are sturdily mounted to the arcade machine, I figured some more mass couldn't hurt.
- The rubber dampeners are 15mm of height and 20mm in diameter. The arcade drum seems to use tapered dampeners, but at least the tapered dampeners I could get my hands on felt too soft, so I went with straight ones.
- For the drum skin I tried with a 2mm natural rubber sheet covered by some canvas, which worked reasonably well but is pretty loud and has little rebound. I imported real arcade drum skins now, which work a lot better.

## Acknowledgements

- [daschr](https://github.com/daschr) for the [SSD1306 OLED driver](https://github.com/daschr/pico-ssd1306)
- [FeralAI](https://github.com/FeralAI) for the inspiration and XInput driver from the [GP2040 Project](https://github.com/FeralAI/GP2040)
- The linux kernel contributors for documenting the game controllers in their drivers
