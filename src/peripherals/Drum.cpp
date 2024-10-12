#include "peripherals/Drum.h"

#include "hardware/adc.h"
#include "pico/time.h"

#include <algorithm>

namespace Doncon::Peripherals {

Drum::InternalAdc::InternalAdc(const Drum::Config::AdcInputs &adc_inputs) {
    adc_gpio_init(adc_inputs.don_left + 26);
    adc_gpio_init(adc_inputs.don_right + 26);
    adc_gpio_init(adc_inputs.ka_left + 26);
    adc_gpio_init(adc_inputs.ka_right + 26);

    adc_init();
}

uint16_t Drum::InternalAdc::read(uint8_t channel) {
    adc_select_input(channel);
    return adc_read();
}

Drum::ExternalAdc::ExternalAdc(const Drum::Config::Spi &spi_config) : m_mcp3204(spi_config.block, spi_config.scsn_pin) {
    // Enable level shifter
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, true);

    gpio_set_function(spi_config.miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_config.mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_config.sclk_pin, GPIO_FUNC_SPI);
    // gpio_set_function(spi_config.scsn_pin, GPIO_FUNC_SPI);

    spi_init(spi_config.block, spi_config.speed_hz);

    // Theoretically the ADC should work in SPI 1,1 mode.
    // In this mode date will be clocked out on
    // a falling edge and latched from the ADC on a rising edge.
    // Also the CS will be held low in-between bytes as required
    // by the mcp3204.
    // However this mode causes glitches during continuous reading,
    // so we need to use default mode and set CS manually.
    // spi_set_format(m_spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_init(spi_config.scsn_pin);
    gpio_set_dir(spi_config.scsn_pin, GPIO_OUT);
    gpio_put(spi_config.scsn_pin, true);
}

uint16_t Drum::ExternalAdc::read(uint8_t channel) { return m_mcp3204.read(channel); }

Drum::Pad::Pad(const uint8_t channel) : channel(channel), last_change(0), active(false) {}

void Drum::Pad::setState(const bool state, const uint16_t debounce_delay) {
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
    if (m_config.use_external_adc) {
        m_adc = std::make_unique<ExternalAdc>(config.external_adc_spi_config);
    } else {
        m_adc = std::make_unique<InternalAdc>(config.adc_inputs);
    }

    m_pads.emplace(Id::DON_LEFT, config.adc_inputs.don_left);
    m_pads.emplace(Id::KA_LEFT, config.adc_inputs.ka_left);
    m_pads.emplace(Id::DON_RIGHT, config.adc_inputs.don_right);
    m_pads.emplace(Id::KA_RIGHT, config.adc_inputs.ka_right);
}

std::map<Drum::Id, uint16_t> Drum::sampleInputs() {
    std::map<Id, uint32_t> values;

    for (uint8_t sample_number = 0; sample_number < m_config.sample_count; ++sample_number) {
        for (const auto &pad : m_pads) {
            values[pad.first] += m_adc->read(pad.second.getChannel());
        }
    }

    // Take average of all samples
    std::map<Id, uint16_t> result;
    for (auto &value : values) {
        result[value.first] = value.second / m_config.sample_count;
    }

    return result;
}

void Drum::updateRollCounter(Utils::InputState &input_state) {
    static uint32_t last_hit_time = 0;
    static bool last_don_left_state = false;
    static bool last_don_right_state = false;
    static bool last_ka_left_state = false;
    static bool last_ka_right_state = false;
    static uint16_t roll_count = 0;
    static uint16_t previous_roll = 0;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - last_hit_time) > m_config.roll_counter_timeout_ms) {
        if (roll_count > 1) {
            previous_roll = roll_count;
        }
        roll_count = 0;
    }

    if (input_state.drum.don_left.triggered && (last_don_left_state != input_state.drum.don_left.triggered)) {
        last_hit_time = now;
        roll_count++;
    }
    if (input_state.drum.don_right.triggered && (last_don_right_state != input_state.drum.don_right.triggered)) {
        last_hit_time = now;
        roll_count++;
    }
    if (input_state.drum.ka_right.triggered && (last_ka_right_state != input_state.drum.ka_right.triggered)) {
        last_hit_time = now;
        roll_count++;
    }
    if (input_state.drum.ka_left.triggered && (last_ka_left_state != input_state.drum.ka_left.triggered)) {
        last_hit_time = now;
        roll_count++;
    }

    last_don_left_state = input_state.drum.don_left.triggered;
    last_don_right_state = input_state.drum.don_right.triggered;
    last_ka_left_state = input_state.drum.ka_left.triggered;
    last_ka_right_state = input_state.drum.ka_right.triggered;

    input_state.drum.current_roll = roll_count;
    input_state.drum.previous_roll = previous_roll;
}

void Drum::updateInputState(Utils::InputState &input_state) {
    // Oversample ADC inputs to get rid of ADC noise
    const auto raw_values = sampleInputs();

    auto get_threshold = [&](const Id target) {
        switch (target) {
        case Id::DON_LEFT:
            return m_config.trigger_thresholds.don_left;
        case Id::DON_RIGHT:
            return m_config.trigger_thresholds.don_right;
        case Id::KA_LEFT:
            return m_config.trigger_thresholds.ka_left;
        case Id::KA_RIGHT:
            return m_config.trigger_thresholds.ka_right;
        case Id::NONE:
            return (uint16_t)0;
        }
        return (uint16_t)0;
    };

    const auto set_max = [&](Id a, Id b) {
        const auto set_with_threshold = [&](Id target) {
            if (raw_values.at(target) > get_threshold(target)) {
                m_pads.at(target).setState(true, m_config.debounce_delay_ms);
            } else {
                m_pads.at(target).setState(false, m_config.debounce_delay_ms);
            }
        };

        if (raw_values.at(a) > raw_values.at(b)) {
            set_with_threshold(a);
            m_pads.at(b).setState(false, m_config.debounce_delay_ms);
        } else {
            set_with_threshold(b);
            m_pads.at(a).setState(false, m_config.debounce_delay_ms);
        }
    };

    // Consider the hardest hit for each side
    set_max(Id::DON_LEFT, Id::KA_LEFT);
    set_max(Id::DON_RIGHT, Id::KA_RIGHT);

    input_state.drum.don_left.raw = raw_values.at(Id::DON_LEFT);
    input_state.drum.ka_left.raw = raw_values.at(Id::KA_LEFT);
    input_state.drum.don_right.raw = raw_values.at(Id::DON_RIGHT);
    input_state.drum.ka_right.raw = raw_values.at(Id::KA_RIGHT);

    input_state.drum.don_left.triggered = m_pads.at(Id::DON_LEFT).getState();
    input_state.drum.ka_left.triggered = m_pads.at(Id::KA_LEFT).getState();
    input_state.drum.don_right.triggered = m_pads.at(Id::DON_RIGHT).getState();
    input_state.drum.ka_right.triggered = m_pads.at(Id::KA_RIGHT).getState();

    updateRollCounter(input_state);
}

void Drum::setDebounceDelay(const uint16_t delay) { m_config.debounce_delay_ms = delay; }

void Drum::setThresholds(const Config::Thresholds &thresholds) { m_config.trigger_thresholds = thresholds; }

} // namespace Doncon::Peripherals
