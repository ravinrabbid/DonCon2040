#ifndef PERIPHERALS_DRUM_H_
#define PERIPHERALS_DRUM_H_

#include "utils/InputState.h"

#include "hardware/spi.h"

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <variant>

namespace Doncon::Peripherals {

class Drum {
  public:
    struct Config {
        struct __attribute((packed, aligned(1))) Thresholds {
            uint16_t don_left;
            uint16_t ka_left;
            uint16_t don_right;
            uint16_t ka_right;
        };

        struct AdcChannels {
            uint8_t don_left;
            uint8_t ka_left;
            uint8_t don_right;
            uint8_t ka_right;
        };

        struct InternalAdc {
            uint8_t sample_count;
        };

        struct ExternalAdc {
            spi_inst_t *spi_block;
            uint spi_speed_hz;
            uint8_t spi_mosi_pin;
            uint8_t spi_miso_pin;
            uint8_t spi_sclk_pin;
            uint8_t spi_scsn_pin;
            uint8_t spi_level_shifter_enable_pin;
        };

        enum class DoubleTriggerMode : uint8_t {
            Off,
            Threshold,
            Always,
        };

        Thresholds trigger_thresholds;

        DoubleTriggerMode double_trigger_mode;
        Thresholds double_trigger_thresholds;

        uint16_t debounce_delay_ms;

        uint32_t roll_counter_timeout_ms;

        AdcChannels adc_channels;
        std::variant<InternalAdc, ExternalAdc> adc_config;
    };

  private:
    enum class Id : uint8_t {
        DON_LEFT,
        KA_LEFT,
        DON_RIGHT,
        KA_RIGHT,
    };

    class Pad {
      private:
        struct analog_buffer_entry {
            uint16_t value;
            uint32_t timestamp;
        };

        uint8_t m_channel;
        uint32_t m_last_change{0};
        bool m_active{false};
        std::deque<analog_buffer_entry> m_analog_buffer;

      public:
        Pad(uint8_t channel);

        [[nodiscard]] uint8_t getChannel() const { return m_channel; };
        [[nodiscard]] bool getState() const { return m_active; };
        void setState(bool state, uint16_t debounce_delay);
        uint16_t getAnalog();
        void setAnalog(uint16_t value, uint16_t debounce_delay);
    };

    class RollCounter {
      private:
        uint32_t m_timeout_ms;

        uint32_t m_last_hit_time{0};
        uint16_t m_current_roll{0};
        uint16_t m_previous_roll{0};

        struct {
            bool don_left;
            bool ka_left;
            bool don_right;
            bool ka_right;
        } m_previous_pad_state{};

      public:
        RollCounter(uint32_t timeout_ms);
        void update(Utils::InputState &input_state);

        [[nodiscard]] uint16_t getCurrentRoll() const { return m_current_roll; };
        [[nodiscard]] uint16_t getPreviousRoll() const { return m_previous_roll; };
    };

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Class has no members
    class AdcInterface {
      public:
        virtual ~AdcInterface() = default;

        // Those are expected to be 12bit values
        virtual std::array<uint16_t, 4> read() = 0;
    };

    class InternalAdc : public AdcInterface {
      private:
        Config::InternalAdc m_config;

      public:
        InternalAdc(const Config::InternalAdc &config);
        std::array<uint16_t, 4> read() final;
    };

    class ExternalAdc : public AdcInterface {
      public:
        ExternalAdc(const Config::ExternalAdc &config);
        std::array<uint16_t, 4> read() final;
    };

    Config m_config;
    std::unique_ptr<AdcInterface> m_adc;
    std::map<Id, Pad> m_pads;
    RollCounter m_roll_counter;

    void updateDigitalInputState(Utils::InputState &input_state, const std::map<Id, uint16_t> &raw_values);
    void updateAnalogInputState(Utils::InputState &input_state, const std::map<Id, uint16_t> &raw_values);
    std::map<Id, uint16_t> readInputs();

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);

    void setDebounceDelay(uint16_t delay);
    void setTriggerThresholds(const Config::Thresholds &thresholds);
    void setDoubleTriggerMode(Config::DoubleTriggerMode mode);
    void setDoubleThresholds(const Config::Thresholds &thresholds);
};

} // namespace Doncon::Peripherals

#endif // PERIPHERALS_DRUM_H_