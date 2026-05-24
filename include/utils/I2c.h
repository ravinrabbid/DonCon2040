#ifndef UTILS_I2C_H_
#define UTILS_I2C_H_

#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include <cstdint>

namespace Doncon::Utils::I2c {

struct Config {
    uint8_t sda_pin;
    uint8_t scl_pin;
    i2c_inst_t *block;
    uint speed_hz;
};

inline void initGpio(const Config &config) {
    gpio_init(config.sda_pin);
    gpio_init(config.scl_pin);
    gpio_set_function(config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config.sda_pin);
    gpio_pull_up(config.scl_pin);
}

} // namespace Doncon::Utils::I2c

#endif // UTILS_I2C_H_
