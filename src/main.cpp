#include "pico/stdlib.h"

#include <stdio.h>

int main() {
    stdio_init_all();

    while (true) {
        sleep_ms(1);
    }
}