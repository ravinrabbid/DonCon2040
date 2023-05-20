#include "peripherals/Buttons.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"
#include "usb/usb_driver.h"

#include "GlobalConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <stdio.h>

using namespace Doncon;

queue_t input_queue;

void core1_task() {
    multicore_lockout_victim_init();

    Utils::InputState input_state;
    Peripherals::StatusLed led(Config::Default::led_config);

    while (true) {
        if (queue_try_remove(&input_queue, &input_state)) {
            led.setInputState(input_state);
        }

        led.update();

        sleep_ms(1);
    }
}

int main() {
    queue_init(&input_queue, sizeof(Utils::InputState), 1);

    Utils::InputState input_state;
    Peripherals::Drum drum(Config::Default::drum_config);
    Peripherals::Buttons buttons(Config::Default::button_config); // Move to core 1?
    usb_mode_t mode = USB_MODE_XBOX360;

    usb_driver_init(mode);

    stdio_init_all();

    multicore_launch_core1(core1_task);

    while (true) {
        drum.updateInputState(input_state);
        buttons.updateInputState(input_state);

        usb_driver_send_and_receive_report(input_state.getReport(mode));
        usb_driver_task();

        queue_try_add(&input_queue, &input_state);
    }

    return 0;
}