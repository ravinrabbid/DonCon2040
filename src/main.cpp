#include "peripherals/Drum.h"

#include "GlobalConfiguration.h"

#include "pico/stdlib.h"

#include <stdio.h>

using namespace Doncon;

int main() {
    Utils::InputState input_state;
    Peripherals::Drum drum(Config::Default::drum_config);

    stdio_init_all();

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
    }

    return 0;
}