#include "peripherals/Drum.h"

#include "hardware/adc.h"
#include "pico/time.h"

#include <algorithm>

namespace Doncon::Peripherals {

Drum::Pad::Pad(uint8_t pin) : pin(pin), last_change(0), active(false) {}

void Drum::Pad::setState(bool state, uint16_t debounce_delay) {
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
    m_pads.emplace(Id::DON_LEFT, config.pins.don_left);
    m_pads.emplace(Id::KA_LEFT, config.pins.ka_left);
    m_pads.emplace(Id::DON_RIGHT, config.pins.don_right);
    m_pads.emplace(Id::KA_RIGHT, config.pins.ka_right);

    adc_init();

    for (const auto &pad : m_pads) {
        adc_gpio_init(pad.second.getPin());
    }
}

void Drum::updateInputState(Utils::InputState &input_state) {
    // Oversample ADC inputs to get rid of ADC noise
    const auto raw_values = sampleInputs();

    for (const auto &val : raw_values) {
        switch (val.first) {
        case Id::DON_LEFT:
            input_state.drum.don_left.raw = val.second;
            if (val.second > m_config.trigger_thresholds.don_left) {
                m_pads.at(Id::DON_LEFT).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(Id::DON_LEFT).setState(false, m_config.debounce_delay_ms);
            }
            break;
        case Id::KA_LEFT:
            input_state.drum.ka_left.raw = val.second;
            if (val.second > m_config.trigger_thresholds.ka_left) {
                m_pads.at(Id::KA_LEFT).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(Id::KA_LEFT).setState(false, m_config.debounce_delay_ms);
            }
            break;
        case Id::DON_RIGHT:
            input_state.drum.don_right.raw = val.second;
            if (val.second > m_config.trigger_thresholds.don_right) {
                m_pads.at(Id::DON_RIGHT).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(Id::DON_RIGHT).setState(false, m_config.debounce_delay_ms);
            }
            break;
        case Id::KA_RIGHT:
            input_state.drum.ka_right.raw = val.second;
            if (val.second > m_config.trigger_thresholds.ka_right) {
                m_pads.at(Id::KA_RIGHT).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(Id::KA_RIGHT).setState(false, m_config.debounce_delay_ms);
            }
            break;
        }
    }

    input_state.drum.don_left.triggered = m_pads.at(Id::DON_LEFT).getState();
    input_state.drum.ka_left.triggered = m_pads.at(Id::KA_LEFT).getState();
    input_state.drum.don_right.triggered = m_pads.at(Id::DON_RIGHT).getState();
    input_state.drum.ka_right.triggered = m_pads.at(Id::KA_RIGHT).getState();
}

std::map<Drum::Id, uint16_t> Drum::sampleInputs() {
    std::map<Id, uint32_t> values;

    for (uint8_t sample_number = 0; sample_number < m_config.sample_count; ++sample_number) {
        for (const auto &pad : m_pads) {
            adc_select_input(pad.second.getPin());
            values[pad.first] += adc_read();
        }
    }

    // Take average of all samples
    std::map<Id, uint16_t> result;
    for (auto &value : values) {
        result[value.first] = value.second / m_config.sample_count;
    }

    return result;
}

} // namespace Doncon::Peripherals
