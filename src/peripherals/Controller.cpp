#include "peripherals/Controller.h"

#include "hardware/gpio.h"
#include "pico/time.h"

namespace Doncon::Peripherals {

Controller::Button::Button(uint8_t pin) : m_gpio_pin(pin), m_gpio_mask(1 << pin) {}

void Controller::Button::setState(bool state, uint8_t debounce_delay) {
    if (m_active == state) {
        return;
    }

    // Immediately change the input state, but only allow a change every debounce_delay milliseconds.
    const uint32_t now = to_ms_since_boot(get_absolute_time());
    if (m_last_change + debounce_delay <= now) {
        m_active = state;
        m_last_change = now;
    }
}

Controller::InternalGpio::InternalGpio(const std::map<Id, Button> &buttons) {
    for (const auto &button : buttons) {
        gpio_init(button.second.getGpioPin());
        gpio_set_dir(button.second.getGpioPin(), (bool)GPIO_IN);
        gpio_pull_up(button.second.getGpioPin());
    }
}

uint32_t Controller::InternalGpio::read() { return ~gpio_get_all(); }

Controller::ExternalGpio::ExternalGpio(const Config::ExternalGpio &config)
    : m_mcp23017(config.i2c.address, config.i2c.block) {
    m_mcp23017.setDirection(0xFFFF);       // All inputs
    m_mcp23017.setPullup(0xFFFF);          // All on
    m_mcp23017.setReversePolarity(0xFFFF); // All reversed
}

uint32_t Controller::ExternalGpio::read() { return m_mcp23017.read(); }

void Controller::socdClean(Utils::InputState &input_state) {

    // Last input has priority
    if (input_state.controller.dpad.up && input_state.controller.dpad.down) {
        if (m_socd_state.lastVertical == Id::DOWN) {
            input_state.controller.dpad.down = false;
        } else if (m_socd_state.lastVertical == Id::UP) {
            input_state.controller.dpad.up = false;
        }
    } else if (input_state.controller.dpad.up) {
        m_socd_state.lastVertical = Id::UP;
    } else {
        m_socd_state.lastVertical = Id::DOWN;
    }

    if (input_state.controller.dpad.left && input_state.controller.dpad.right) {
        if (m_socd_state.lastHorizontal == Id::RIGHT) {
            input_state.controller.dpad.right = false;
        } else if (m_socd_state.lastHorizontal == Id::LEFT) {
            input_state.controller.dpad.left = false;
        }
    } else if (input_state.controller.dpad.left) {
        m_socd_state.lastHorizontal = Id::LEFT;
    } else {
        m_socd_state.lastHorizontal = Id::RIGHT;
    }
}

Controller::Controller(const Config &config) : m_config(config) {
    m_buttons.emplace(Id::UP, config.pins.dpad.up);
    m_buttons.emplace(Id::DOWN, config.pins.dpad.down);
    m_buttons.emplace(Id::LEFT, config.pins.dpad.left);
    m_buttons.emplace(Id::RIGHT, config.pins.dpad.right);
    m_buttons.emplace(Id::NORTH, config.pins.buttons.north);
    m_buttons.emplace(Id::EAST, config.pins.buttons.east);
    m_buttons.emplace(Id::SOUTH, config.pins.buttons.south);
    m_buttons.emplace(Id::WEST, config.pins.buttons.west);
    m_buttons.emplace(Id::L, config.pins.buttons.l);
    m_buttons.emplace(Id::R, config.pins.buttons.r);
    m_buttons.emplace(Id::START, config.pins.buttons.start);
    m_buttons.emplace(Id::SELECT, config.pins.buttons.select);
    m_buttons.emplace(Id::HOME, config.pins.buttons.home);
    m_buttons.emplace(Id::SHARE, config.pins.buttons.share);

    std::visit(
        [this](auto &&config) {
            using T = std::decay_t<decltype(config)>;

            if constexpr (std::is_same_v<T, Config::InternalGpio>) {
                m_gpio = std::make_unique<InternalGpio>(m_buttons);
            } else if constexpr (std::is_same_v<T, Config::ExternalGpio>) {
                m_gpio = std::make_unique<ExternalGpio>(config);
            } else {
                static_assert(false, "Unknown GPIO type!");
            }
        },
        m_config.gpio_config);
}

void Controller::updateInputState(Utils::InputState &input_state) {
    const uint32_t gpio_state = m_gpio->read();

    for (auto &button : m_buttons) {
        button.second.setState((gpio_state & button.second.getGpioMask()) != 0, m_config.debounce_delay_ms);
    }

    input_state.controller.dpad.up = m_buttons.at(Id::UP).getState();
    input_state.controller.dpad.down = m_buttons.at(Id::DOWN).getState();
    input_state.controller.dpad.left = m_buttons.at(Id::LEFT).getState();
    input_state.controller.dpad.right = m_buttons.at(Id::RIGHT).getState();
    input_state.controller.buttons.north = m_buttons.at(Id::NORTH).getState();
    input_state.controller.buttons.east = m_buttons.at(Id::EAST).getState();
    input_state.controller.buttons.south = m_buttons.at(Id::SOUTH).getState();
    input_state.controller.buttons.west = m_buttons.at(Id::WEST).getState();
    input_state.controller.buttons.l = m_buttons.at(Id::L).getState();
    input_state.controller.buttons.r = m_buttons.at(Id::R).getState();
    input_state.controller.buttons.start = m_buttons.at(Id::START).getState();
    input_state.controller.buttons.select = m_buttons.at(Id::SELECT).getState();
    input_state.controller.buttons.home = m_buttons.at(Id::HOME).getState();
    input_state.controller.buttons.share = m_buttons.at(Id::SHARE).getState();

    socdClean(input_state);
}
} // namespace Doncon::Peripherals
