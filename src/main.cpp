#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"

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

    stdio_init_all();

    multicore_launch_core1(core1_task);

    while (true) {
        drum.updateInputState(input_state);

        if (input_state.drum.don_left) {
            printf("DON LEFT\n");
        }
        if (input_state.drum.don_right) {
            printf("DON RIGHT\n");
        }
        if (input_state.drum.ka_left) {
            printf("KA LEFT\n");
        }
        if (input_state.drum.ka_right) {
            printf("KA RIGHT\n");
        }

        queue_try_add(&input_queue, &input_state);
    }

    return 0;
}