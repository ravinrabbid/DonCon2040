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

Drum::Pad::Pad(uint8_t channel) : channel(channel), last_change(0), active(false) {}

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

void Drum::setThresholds(const Config::Thresholds &thresholds) { m_config.trigger_thresholds = thresholds; }

void Drum::setThresholdScaleLevel(const uint8_t threshold_scale_level) {
    m_config.trigger_threshold_scale_level = threshold_scale_level;
}

} // namespace Doncon::Peripherals
