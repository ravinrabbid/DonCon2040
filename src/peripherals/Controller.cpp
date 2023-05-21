#include "peripherals/Controller.h"

#include "pico/time.h"

namespace Doncon::Peripherals {

Buttons::Button::Button(uint8_t pin) : gpio_pin(pin), gpio_mask(1 << pin), last_change(0), active(false) {}

void Buttons::Button::setState(bool state, uint8_t debounce_delay) {
    if (active == state) {
        return;
    }

    // Immediately change the input state, but only allow a change every debounce_delay milliseconds.
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (last_change + debounce_delay <= now) {
        active = state;
        last_change = now;
    }
}

void Buttons::socdClean(Utils::InputState &input_state) {

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

Buttons::Buttons(const Config &config) : m_config(config), m_socd_state{Id::DOWN, Id::RIGHT} {
    m_mcp23017 = std::make_unique<Mcp23017>(m_config.i2c.address, m_config.i2c.block);
    m_mcp23017->setDirection(0xFFFF);       // All inputs
    m_mcp23017->setPullup(0xFFFF);          // All on
    m_mcp23017->setReversePolarity(0xFFFF); // All reversed

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
}

void Buttons::updateInputState(Utils::InputState &input_state) {
    uint16_t gpio_state = m_mcp23017->read();

    for (auto &button : m_buttons) {
        button.second.setState(gpio_state & button.second.getGpioMask(), m_config.debounce_delay_ms);
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
