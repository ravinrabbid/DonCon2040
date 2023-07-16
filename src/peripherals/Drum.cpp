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

    const auto max_value = std::max_element(raw_values.cbegin(), raw_values.cend(),
                                            [](const auto a, const auto b) { return a.second < b.second; });

    auto do_set_state = [&](const auto &id, const auto &threshold) {
        // Increase threshold for very hard hits to avoid false inputs on neighboring pads
        float mult = 1.0;
        if (id != max_value->first && m_config.trigger_threshold_scale_level > 0) {
            mult = float(max_value->second) / (((UINT8_MAX * 16) - (m_config.trigger_threshold_scale_level * 16)) + 1);
            mult = mult < 1.0 ? 1.0 : mult;
        }

        if (raw_values.at(id) > (threshold * mult)) {
            m_pads.at(id).setState(true, m_config.debounce_delay_ms);
        } else {
            m_pads.at(id).setState(false, m_config.debounce_delay_ms);
        }
    };

    do_set_state(Id::DON_LEFT, m_config.trigger_thresholds.don_left);
    do_set_state(Id::KA_LEFT, m_config.trigger_thresholds.ka_left);
    do_set_state(Id::DON_RIGHT, m_config.trigger_thresholds.don_right);
    do_set_state(Id::KA_RIGHT, m_config.trigger_thresholds.ka_right);

    input_state.drum.don_left.raw = raw_values.at(Id::DON_LEFT);
    input_state.drum.ka_left.raw = raw_values.at(Id::KA_LEFT);
    input_state.drum.don_right.raw = raw_values.at(Id::DON_RIGHT);
    input_state.drum.ka_right.raw = raw_values.at(Id::KA_RIGHT);

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
