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
    const auto raw_values = sampleInputs(5, 100);

    for (const auto &val : raw_values) {
        switch (val.first) {
        case Id::DON_LEFT:
            input_state.drum.don_left.raw = val.second;
            break;
        case Id::KA_LEFT:
            input_state.drum.ka_left.raw = val.second;
            break;
        case Id::DON_RIGHT:
            input_state.drum.don_right.raw = val.second;
            break;
        case Id::KA_RIGHT:
            input_state.drum.ka_right.raw = val.second;
            break;
        }
    }

    auto hardest_hit = *std::max_element(raw_values.begin(), raw_values.end(),
                                         [](const auto a, const auto b) { return a.second < b.second; });

    if (hardest_hit.second > m_config.trigger_threshold) {
        m_pads.at(hardest_hit.first).setState(true, m_config.debounce_delay_ms);

        auto set_twin = [&](const Id twin) {
            if ((raw_values.at(twin) > m_config.trigger_threshold) &&
                std::abs(static_cast<int32_t>(hardest_hit.second) - raw_values.at(twin)) <
                    m_config.double_hit_threshold) {

                m_pads.at(twin).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(twin).setState(false, m_config.debounce_delay_ms);
            }
        };

        switch (hardest_hit.first) {
        case Id::DON_LEFT:
            set_twin(Id::DON_RIGHT);
            m_pads.at(Id::KA_LEFT).setState(false, m_config.debounce_delay_ms);
            m_pads.at(Id::KA_RIGHT).setState(false, m_config.debounce_delay_ms);
            break;
        case Id::KA_LEFT:
            set_twin(Id::KA_RIGHT);
            m_pads.at(Id::DON_LEFT).setState(false, m_config.debounce_delay_ms);
            m_pads.at(Id::DON_RIGHT).setState(false, m_config.debounce_delay_ms);
            break;
        case Id::DON_RIGHT:
            set_twin(Id::DON_LEFT);
            m_pads.at(Id::KA_LEFT).setState(false, m_config.debounce_delay_ms);
            m_pads.at(Id::KA_RIGHT).setState(false, m_config.debounce_delay_ms);
            break;
        case Id::KA_RIGHT:
            set_twin(Id::KA_LEFT);
            m_pads.at(Id::DON_LEFT).setState(false, m_config.debounce_delay_ms);
            m_pads.at(Id::DON_RIGHT).setState(false, m_config.debounce_delay_ms);
            break;
        }
    } else {
        m_pads.at(Id::DON_LEFT).setState(false, m_config.debounce_delay_ms);
        m_pads.at(Id::DON_RIGHT).setState(false, m_config.debounce_delay_ms);
        m_pads.at(Id::KA_LEFT).setState(false, m_config.debounce_delay_ms);
        m_pads.at(Id::KA_RIGHT).setState(false, m_config.debounce_delay_ms);
    }

    input_state.drum.don_left.triggered = m_pads.at(Id::DON_LEFT).getState();
    input_state.drum.ka_left.triggered = m_pads.at(Id::KA_LEFT).getState();
    input_state.drum.don_right.triggered = m_pads.at(Id::DON_RIGHT).getState();
    input_state.drum.ka_right.triggered = m_pads.at(Id::KA_RIGHT).getState();
}

std::map<Drum::Id, uint16_t> Drum::sampleInputs(uint8_t count, uint16_t delay_us) {
    std::map<Id, uint32_t> values;

    delay_us = (delay_us - 8) > 0 ? (delay_us - 8) : 0; // Each sample takes 4 * 2µs

    for (uint8_t sample_number = 0; sample_number < count; ++sample_number) {
        for (const auto &pad : m_pads) {
            adc_select_input(pad.second.getPin());
            values[pad.first] += adc_read();
        }

        sleep_us(delay_us);
    }

    // Take average of all samples
    std::map<Id, uint16_t> result;
    for (auto &value : values) {
        result[value.first] = value.second / count;
    }

    return result;
}

} // namespace Doncon::Peripherals
