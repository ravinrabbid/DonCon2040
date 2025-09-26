#include "peripherals/Drum.h"

#include "hardware/adc.h"
#include "pico/time.h"
#include <mcp3204/Mcp3204Dma.h>

#include <algorithm>

namespace Doncon::Peripherals {

Drum::InternalAdc::InternalAdc(const Config::InternalAdc &config) : m_config(config) {
    static const uint adc_base_pin = 26;

    for (uint pin = adc_base_pin; pin < adc_base_pin + 4; ++pin) {
        adc_gpio_init(pin);
    }

    adc_init();
}

std::array<uint16_t, 4> Drum::InternalAdc::read() {
    if (m_config.sample_count == 0) {
        return {};
    }

    // Oversample ADC inputs to get rid of ADC noise
    std::array<uint32_t, 4> values{};
    for (uint8_t sample_number = 0; sample_number < m_config.sample_count; ++sample_number) {
        for (size_t idx = 0; idx < values.size(); ++idx) {
            adc_select_input(idx);
            values.at(idx) += adc_read();
        }
    }

    // Take average of all samples
    std::array<uint16_t, 4> result{};
    std::ranges::transform(values, result.begin(), [&](const auto &sample) { return sample / m_config.sample_count; });

    return result;
}

Drum::ExternalAdc::ExternalAdc(const Config::ExternalAdc &config) {
    // Enable level shifter
    gpio_init(config.spi_level_shifter_enable_pin);
    gpio_set_dir(config.spi_level_shifter_enable_pin, (bool)GPIO_OUT);
    gpio_put(config.spi_level_shifter_enable_pin, true);

    // Set up SPI
    gpio_set_function(config.spi_miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(config.spi_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(config.spi_sclk_pin, GPIO_FUNC_SPI);
    spi_init(config.spi_block, config.spi_speed_hz);

    gpio_init(config.spi_scsn_pin);
    gpio_set_dir(config.spi_scsn_pin, (bool)GPIO_OUT);

    Mcp3204Dma::run(config.spi_block, config.spi_scsn_pin);
}

std::array<uint16_t, 4> Drum::ExternalAdc::read() { return Mcp3204Dma::take_maximums(); }

Drum::Pad::Pad(const uint8_t channel) : m_channel(channel) {}

void Drum::Pad::setState(const bool state, const uint16_t debounce_delay) {
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

uint16_t Drum::Pad::getAnalog() {
    const auto raw_to_uint16 = [](uint16_t raw) { return ((raw << 4) & 0xFFF0) | ((raw >> 8) & 0x000F); };

    return raw_to_uint16(std::ranges::max_element(m_analog_buffer, [](const auto &a, const auto &b) {
                             return a.value < b.value;
                         })->value);
}

void Drum::Pad::setAnalog(uint16_t value, uint16_t debounce_delay) {
    const uint32_t now = to_ms_since_boot(get_absolute_time());

    // Clear outdated values, i.e. anything older than debounce_delay to allow for convenient configuration.
    while (!m_analog_buffer.empty() && (m_analog_buffer.front().timestamp + debounce_delay) <= now) {
        m_analog_buffer.pop_front();
    }

    m_analog_buffer.push_back({value, now});
}

Drum::RollCounter::RollCounter(uint32_t timeout_ms) : m_timeout_ms(timeout_ms) {};

void Drum::RollCounter::update(Utils::InputState &input_state) {
    const uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - m_last_hit_time) > m_timeout_ms) {
        if (m_current_roll > 1) {
            m_previous_roll = m_current_roll;
        }
        m_current_roll = 0;
    }

    if (input_state.drum.don_left.triggered && (m_previous_pad_state.don_left != input_state.drum.don_left.triggered)) {
        m_last_hit_time = now;
        m_current_roll++;
    }
    if (input_state.drum.don_right.triggered &&
        (m_previous_pad_state.don_right != input_state.drum.don_right.triggered)) {
        m_last_hit_time = now;
        m_current_roll++;
    }
    if (input_state.drum.ka_right.triggered && (m_previous_pad_state.ka_right != input_state.drum.ka_right.triggered)) {
        m_last_hit_time = now;
        m_current_roll++;
    }
    if (input_state.drum.ka_left.triggered && (m_previous_pad_state.ka_left != input_state.drum.ka_left.triggered)) {
        m_last_hit_time = now;
        m_current_roll++;
    }

    m_previous_pad_state.don_left = input_state.drum.don_left.triggered;
    m_previous_pad_state.don_right = input_state.drum.don_right.triggered;
    m_previous_pad_state.ka_left = input_state.drum.ka_left.triggered;
    m_previous_pad_state.ka_right = input_state.drum.ka_right.triggered;

    input_state.drum.current_roll = m_current_roll;
    input_state.drum.previous_roll = m_previous_roll;
}

Drum::Drum(const Config &config) : m_config(config), m_roll_counter(config.roll_counter_timeout_ms) {

    std::visit(
        [this](auto &&config) {
            using T = std::decay_t<decltype(config)>;

            if constexpr (std::is_same_v<T, Config::InternalAdc>) {
                m_adc = std::make_unique<InternalAdc>(config);
            } else if constexpr (std::is_same_v<T, Config::ExternalAdc>) {
                m_adc = std::make_unique<ExternalAdc>(config);
            } else {
                static_assert(false, "Unknown ADC type!");
            }
        },
        m_config.adc_config);

    m_pads.emplace(Id::DON_LEFT, config.adc_channels.don_left);
    m_pads.emplace(Id::KA_LEFT, config.adc_channels.ka_left);
    m_pads.emplace(Id::DON_RIGHT, config.adc_channels.don_right);
    m_pads.emplace(Id::KA_RIGHT, config.adc_channels.ka_right);
}

std::map<Drum::Id, uint16_t> Drum::readInputs() {
    std::map<Id, uint16_t> result;

    const auto adc_values = m_adc->read();

    for (const auto &[id, pad] : m_pads) {
        result[id] = adc_values.at(pad.getChannel());
    }

    return result;
}

void Drum::updateDigitalInputState(Utils::InputState &input_state, const std::map<Drum::Id, uint16_t> &raw_values) {
    const auto resolve_twin_pads = [&](Id left, Id right) {
        const auto is_over_threshold = [&raw_values](const Id target, const auto &thresholds) {
            const auto get_threshold = [&thresholds](const Id target) {
                switch (target) {
                case Id::DON_LEFT:
                    return thresholds.don_left;
                case Id::DON_RIGHT:
                    return thresholds.don_right;
                case Id::KA_LEFT:
                    return thresholds.ka_left;
                case Id::KA_RIGHT:
                    return thresholds.ka_right;
                }
                assert(false);
                return (uint16_t)0;
            };
            return (raw_values.at(target) > get_threshold(target));
        };

        const auto resolve_single_trigger = [&]() {
            if (!is_over_threshold(left, m_config.trigger_thresholds) &&
                !is_over_threshold(right, m_config.trigger_thresholds)) {

                m_pads.at(left).setState(false, m_config.debounce_delay_ms);
                m_pads.at(right).setState(false, m_config.debounce_delay_ms);
                return;
            }

            // Trigger twin pad if within 50% of hit strength to allow
            // simultaneous hits while still rejecting unintended double hits.
            if (raw_values.at(left) > raw_values.at(right)) {
                m_pads.at(left).setState(true, m_config.debounce_delay_ms);

                if (raw_values.at(right) > (raw_values.at(left) >> 1)) {
                    m_pads.at(right).setState(true, m_config.debounce_delay_ms);
                } else {
                    m_pads.at(right).setState(false, m_config.debounce_delay_ms);
                }
            } else {
                m_pads.at(right).setState(true, m_config.debounce_delay_ms);

                if (raw_values.at(left) > (raw_values.at(right) >> 1)) {
                    m_pads.at(left).setState(true, m_config.debounce_delay_ms);
                } else {
                    m_pads.at(left).setState(false, m_config.debounce_delay_ms);
                }
            }
        };

        switch (m_config.double_trigger_mode) {
        case Config::DoubleTriggerMode::Off:
            resolve_single_trigger();
            break;
        case Config::DoubleTriggerMode::Threshold:
            if (is_over_threshold(left, m_config.double_trigger_thresholds) ||
                is_over_threshold(right, m_config.double_trigger_thresholds)) {

                m_pads.at(left).setState(true, m_config.debounce_delay_ms);
                m_pads.at(right).setState(true, m_config.debounce_delay_ms);
            } else {
                resolve_single_trigger();
            }
            break;
        case Config::DoubleTriggerMode::Always:
            if (is_over_threshold(left, m_config.trigger_thresholds) ||
                is_over_threshold(right, m_config.trigger_thresholds)) {

                m_pads.at(left).setState(true, m_config.debounce_delay_ms);
                m_pads.at(right).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(left).setState(false, m_config.debounce_delay_ms);
                m_pads.at(right).setState(false, m_config.debounce_delay_ms);
            }
            break;
        }
    };

    // Either DON or KA can be active at a time
    if (std::max(raw_values.at(Id::DON_LEFT), raw_values.at(Id::DON_RIGHT)) >
        std::max(raw_values.at(Id::KA_LEFT), raw_values.at(Id::KA_RIGHT))) {

        resolve_twin_pads(Id::DON_LEFT, Id::DON_RIGHT);

        m_pads.at(Id::KA_LEFT).setState(false, m_config.debounce_delay_ms);
        m_pads.at(Id::KA_RIGHT).setState(false, m_config.debounce_delay_ms);
    } else {
        resolve_twin_pads(Id::KA_LEFT, Id::KA_RIGHT);

        m_pads.at(Id::DON_LEFT).setState(false, m_config.debounce_delay_ms);
        m_pads.at(Id::DON_RIGHT).setState(false, m_config.debounce_delay_ms);
    }

    input_state.drum.don_left.triggered = m_pads.at(Id::DON_LEFT).getState();
    input_state.drum.ka_left.triggered = m_pads.at(Id::KA_LEFT).getState();
    input_state.drum.don_right.triggered = m_pads.at(Id::DON_RIGHT).getState();
    input_state.drum.ka_right.triggered = m_pads.at(Id::KA_RIGHT).getState();

    m_roll_counter.update(input_state);
}

void Drum::updateAnalogInputState(Utils::InputState &input_state, const std::map<Drum::Id, uint16_t> &raw_values) {
    for (const auto &[id, raw] : raw_values) {
        m_pads.at(id).setAnalog(raw, m_config.debounce_delay_ms);

        switch (id) {
        case Id::DON_LEFT:
            input_state.drum.don_left.analog = m_pads.at(id).getAnalog();
            break;
        case Id::DON_RIGHT:
            input_state.drum.don_right.analog = m_pads.at(id).getAnalog();
            break;
        case Id::KA_LEFT:
            input_state.drum.ka_left.analog = m_pads.at(id).getAnalog();
            break;
        case Id::KA_RIGHT:
            input_state.drum.ka_right.analog = m_pads.at(id).getAnalog();
            break;
        }
    };
}

void Drum::updateInputState(Utils::InputState &input_state) {
    const auto raw_values = readInputs();

    input_state.drum.don_left.raw = raw_values.at(Id::DON_LEFT);
    input_state.drum.don_right.raw = raw_values.at(Id::DON_RIGHT);
    input_state.drum.ka_left.raw = raw_values.at(Id::KA_LEFT);
    input_state.drum.ka_right.raw = raw_values.at(Id::KA_RIGHT);

    updateDigitalInputState(input_state, raw_values);
    updateAnalogInputState(input_state, raw_values);
}

void Drum::setDebounceDelay(const uint16_t delay) { m_config.debounce_delay_ms = delay; }

void Drum::setTriggerThresholds(const Config::Thresholds &thresholds) { m_config.trigger_thresholds = thresholds; }

void Drum::setDoubleTriggerMode(const Config::DoubleTriggerMode mode) { m_config.double_trigger_mode = mode; }

void Drum::setDoubleThresholds(const Config::Thresholds &thresholds) {
    m_config.double_trigger_thresholds = thresholds;
}

} // namespace Doncon::Peripherals
