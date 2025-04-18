#include "peripherals/Drum.h"

#include "hardware/adc.h"
#include "pico/time.h"

#include <algorithm>
#include <deque>

namespace Doncon::Peripherals {

Drum::InternalAdc::InternalAdc(const Config::InternalAdc &config) : m_config(config) {
    static const uint adc_base_pin = 26;

    for (uint pin = adc_base_pin; pin < adc_base_pin + 4; ++pin) {
        adc_gpio_init(pin);
    }

    adc_init();
}

std::array<uint16_t, 4> Drum::InternalAdc::read() {
    // Oversample ADC inputs to get rid of ADC noise
    std::array<uint32_t, 4> values{};
    for (uint8_t sample_number = 0; sample_number < m_config.sample_count; ++sample_number) {
        for (size_t idx = 0; idx < values.size(); ++idx) {
            adc_select_input(idx);
            values[idx] += adc_read();
        }
    }

    // Take average of all samples
    std::array<uint16_t, 4> result{};
    for (size_t idx = 0; idx < values.size(); ++idx) {
        result[idx] = values[idx] / m_config.sample_count;
    }

    return result;
}

Drum::ExternalAdc::ExternalAdc(const Config::ExternalAdc &config) : m_mcp3204(config.spi_block, config.spi_scsn_pin) {
    // Enable level shifter
    gpio_init(config.spi_level_shifter_enable_pin);
    gpio_set_dir(config.spi_level_shifter_enable_pin, GPIO_OUT);
    gpio_put(config.spi_level_shifter_enable_pin, true);

    // Set up SPI
    gpio_set_function(config.spi_miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(config.spi_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(config.spi_sclk_pin, GPIO_FUNC_SPI);
    spi_init(config.spi_block, config.spi_speed_hz);

    gpio_init(config.spi_scsn_pin);
    gpio_set_dir(config.spi_scsn_pin, GPIO_OUT);

    m_mcp3204.run();
}

std::array<uint16_t, 4> Drum::ExternalAdc::read() { return m_mcp3204.take_maximums(); }

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

    for (const auto &pad : m_pads) {
        result[pad.first] = adc_values[pad.second.getChannel()];
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

void Drum::updateDigitalInputState(Utils::InputState &input_state, const std::map<Drum::Id, uint16_t> &raw_values) {

    std::map<Drum::Id, uint16_t> filtered_raw_values;

    // First zero everything below its threshold.
    const auto value_if_above_threshold = [](const auto &values, const auto &thresholds, Id target) {
        const auto get_threshold = [&](const Id target) {
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
        return (values.at(target) > get_threshold(target)) ? values.at(target) : (uint16_t)0;
    };

    for (const auto &entry : raw_values) {
        filtered_raw_values.insert(
            {entry.first, value_if_above_threshold(raw_values, m_config.trigger_thresholds, entry.first)});
    }

    // Only DON or KA can be active at a time, zero the lesser
    if (std::max(filtered_raw_values.at(Id::DON_LEFT), filtered_raw_values.at(Id::DON_RIGHT)) >
        std::max(filtered_raw_values.at(Id::KA_LEFT), filtered_raw_values.at(Id::KA_RIGHT))) {

        filtered_raw_values.at(Id::KA_LEFT) = 0;
        filtered_raw_values.at(Id::KA_RIGHT) = 0;
    } else {
        filtered_raw_values.at(Id::DON_LEFT) = 0;
        filtered_raw_values.at(Id::DON_RIGHT) = 0;
    }

    // Check same same with regard to current debounce state
    if (m_pads.at(Id::DON_LEFT).getState() || m_pads.at(Id::DON_RIGHT).getState()) {
        filtered_raw_values.at(Id::KA_LEFT) = 0;
        filtered_raw_values.at(Id::KA_RIGHT) = 0;
    } else if (m_pads.at(Id::KA_LEFT).getState() || m_pads.at(Id::KA_RIGHT).getState()) {
        filtered_raw_values.at(Id::DON_LEFT) = 0;
        filtered_raw_values.at(Id::DON_RIGHT) = 0;
    }

    // Zero values which are not within +/- 50% of their twin pad
    const auto zero_if_not_within_twin = [](auto &values, Id a, Id b) {
        if (values.at(a) == 0 || values.at(b) == 0) {
            return;
        }

        if (values.at(a) > values.at(b)) {
            if (values.at(b) < (values.at(a) >> 1)) {
                values.at(b) = 0;
            }
        } else {
            if (values.at(a) < (values.at(b) >> 1)) {
                values.at(a) = 0;
            }
        }
    };

    zero_if_not_within_twin(filtered_raw_values, Id::DON_LEFT, Id::DON_RIGHT);
    zero_if_not_within_twin(filtered_raw_values, Id::KA_LEFT, Id::KA_RIGHT);

    // All values != 0 are already over their threshold.
    for (const auto &entry : filtered_raw_values) {
        if (entry.second != 0) {
            m_pads.at(entry.first).setState(true, m_config.debounce_delay_ms);
        } else {
            m_pads.at(entry.first).setState(false, m_config.debounce_delay_ms);
        }
    }

    input_state.drum.don_left.triggered = m_pads.at(Id::DON_LEFT).getState();
    input_state.drum.ka_left.triggered = m_pads.at(Id::KA_LEFT).getState();
    input_state.drum.don_right.triggered = m_pads.at(Id::DON_RIGHT).getState();
    input_state.drum.ka_right.triggered = m_pads.at(Id::KA_RIGHT).getState();

    updateRollCounter(input_state);
}

void Drum::updateAnalogInputState(Utils::InputState &input_state, const std::map<Drum::Id, uint16_t> &raw_values) {
    struct buffer_entry {
        uint16_t value;
        uint32_t timestamp;
    };

    static std::map<Id, std::deque<buffer_entry>> buffer;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    std::for_each(raw_values.cbegin(), raw_values.cend(), [&](const auto &entry) {
        const auto &id = entry.first;
        const auto &raw = entry.second;
        auto &buf = buffer[id];

        // Clear outdated values, i.e. anything older than debounce_delay to allow for convenient configuration.
        while (!buf.empty() && (buf.front().timestamp + m_config.debounce_delay_ms) <= now) {
            buf.pop_front();
        }

        // Insert current value.
        buf.push_back({raw, now});

        // Set maximum value for each pads buffer window.
        const auto get_max = [](const auto &input) {
            return std::max_element(input.cbegin(), input.cend(),
                                    [](const auto &a, const auto &b) { return a.value < b.value; })
                ->value;
        };

        // Map 12bit raw value to 16bit
        const auto raw_to_uint16 = [](uint16_t raw) { return ((raw << 4) & 0xFFF0) | ((raw >> 8) & 0x000F); };

        switch (id) {
        case Id::DON_LEFT:
            input_state.drum.don_left.analog = raw_to_uint16(get_max(buf));
            break;
        case Id::DON_RIGHT:
            input_state.drum.don_right.analog = raw_to_uint16(get_max(buf));
            break;
        case Id::KA_LEFT:
            input_state.drum.ka_left.analog = raw_to_uint16(get_max(buf));
            break;
        case Id::KA_RIGHT:
            input_state.drum.ka_right.analog = raw_to_uint16(get_max(buf));
            break;
        }
    });
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

void Drum::setThresholds(const Config::Thresholds &thresholds) { m_config.trigger_thresholds = thresholds; }

} // namespace Doncon::Peripherals
