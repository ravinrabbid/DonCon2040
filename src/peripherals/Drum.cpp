#include "peripherals/Drum.h"

#include "hardware/gpio.h"
#include "pico/time.h"

namespace Doncon::Peripherals {

Drum::Pad::Pad(uint8_t pin) : gpio_pin(pin), gpio_mask(1 << pin), last_change(0), active(false) {}

void Drum::Pad::setState(bool state, uint8_t debounce_delay) {
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

Drum::Drum(const Config &config) : m_config(config) {
    m_pads.emplace(Id::DON_LEFT_WEAK, config.pins.don_left_weak);
    m_pads.emplace(Id::KA_LEFT_WEAK, config.pins.ka_left_weak);
    m_pads.emplace(Id::DON_RIGHT_WEAK, config.pins.don_right_weak);
    m_pads.emplace(Id::KA_RIGHT_WEAK, config.pins.ka_right_weak);
    m_pads.emplace(Id::DON_LEFT_STRONG, config.pins.don_left_strong);
    m_pads.emplace(Id::KA_LEFT_STRONG, config.pins.ka_left_strong);
    m_pads.emplace(Id::DON_RIGHT_STRONG, config.pins.don_right_strong);
    m_pads.emplace(Id::KA_RIGHT_STRONG, config.pins.ka_right_strong);

    for (const auto &button : m_pads) {
        gpio_init(button.second.getGpioPin());
        gpio_set_dir(button.second.getGpioPin(), GPIO_IN);
        gpio_pull_up(button.second.getGpioPin());
    }
}

void Drum::updateInputState(Utils::InputState &input_state) {
    uint32_t gpio_state = ~gpio_get_all();

    for (auto &button : m_pads) {
        button.second.setState(gpio_state & button.second.getGpioMask(), m_config.debounce_delay_ms);
    }

    input_state.drum.don_left = m_pads.at(Id::DON_LEFT_WEAK).getState() ||   //
                                m_pads.at(Id::DON_LEFT_STRONG).getState() || //
                                m_pads.at(Id::DON_RIGHT_STRONG).getState();

    input_state.drum.ka_left = m_pads.at(Id::KA_LEFT_WEAK).getState() ||   //
                               m_pads.at(Id::KA_LEFT_STRONG).getState() || //
                               m_pads.at(Id::KA_RIGHT_STRONG).getState();

    input_state.drum.don_right = m_pads.at(Id::DON_RIGHT_WEAK).getState() ||  //
                                 m_pads.at(Id::DON_LEFT_STRONG).getState() || //
                                 m_pads.at(Id::DON_RIGHT_STRONG).getState();

    input_state.drum.ka_right = m_pads.at(Id::KA_RIGHT_WEAK).getState() ||  //
                                m_pads.at(Id::KA_LEFT_STRONG).getState() || //
                                m_pads.at(Id::KA_RIGHT_STRONG).getState();
}
} // namespace Doncon::Peripherals
