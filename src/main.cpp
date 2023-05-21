#include "peripherals/Controller.h"
#include "peripherals/Display.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"
#include "usb/usb_driver.h"

#include "GlobalConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <stdio.h>

using namespace Doncon;

queue_t drum_input_queue;
queue_t controller_input_queue;

void core1_task() {
    multicore_lockout_victim_init();

    gpio_set_function(Config::Default::i2c_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(Config::Default::i2c_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(Config::Default::i2c_config.sda_pin);
    gpio_pull_up(Config::Default::i2c_config.scl_pin);
    i2c_init(Config::Default::i2c_config.block, Config::Default::i2c_config.speed_hz);

    Utils::InputState input_state;
    Peripherals::Buttons buttons(Config::Default::button_config);
    Peripherals::StatusLed led(Config::Default::led_config);
    Peripherals::Display display(Config::Default::display_config);

    while (true) {
        buttons.updateInputState(input_state);

        queue_try_add(&controller_input_queue, &input_state.controller);

        if (queue_try_remove(&drum_input_queue, &input_state.drum)) {
            led.setInputState(input_state);
        }

        led.update();
        display.update();

        // sleep_ms(1);
    }
}

int main() {
    queue_init(&drum_input_queue, sizeof(Utils::InputState::Drum), 1);
    queue_init(&controller_input_queue, sizeof(Utils::InputState::Controller), 1);

    Utils::InputState input_state;
    Peripherals::Drum drum(Config::Default::drum_config);
    usb_mode_t mode = USB_MODE_XBOX360;

    usb_driver_init(mode);

    stdio_init_all();

    multicore_launch_core1(core1_task);

    while (true) {
        drum.updateInputState(input_state);

        queue_try_remove(&controller_input_queue, &input_state.controller);

        usb_driver_send_and_receive_report(input_state.getReport(mode));
        usb_driver_task();

        queue_try_add(&drum_input_queue, &input_state);
    }

    return 0;
}