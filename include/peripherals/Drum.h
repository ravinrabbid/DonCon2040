#ifndef _PERIPHERALS_DRUM_H_
#define _PERIPHERALS_DRUM_H_

#include "utils/InputState.h"

#include "hardware/spi.h"

#include <mcp3204/Mcp3204Dma.h>

#include <array>
#include <map>
#include <memory>
#include <stdint.h>
#include <variant>

namespace Doncon::Peripherals {

class Drum {
  public:
    struct Config {
        struct Thresholds {
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

        enum class DoubleTriggerMode {
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
    enum class Id {
        DON_LEFT,
        KA_LEFT,
        DON_RIGHT,
        KA_RIGHT,
    };

    class Pad {
      private:
        uint8_t channel;
        uint32_t last_change;
        bool active;

      public:
        Pad(const uint8_t channel);

        uint8_t getChannel() const { return channel; };
        bool getState() const { return active; };
        void setState(const bool state, const uint16_t debounce_delay);
    };

    class AdcInterface {
      public:
        // Those are expected to be 12bit values
        virtual std::array<uint16_t, 4> read() = 0;
    };

    class InternalAdc : public AdcInterface {
      private:
        Config::InternalAdc m_config;

      public:
        InternalAdc(const Config::InternalAdc &config);
        virtual std::array<uint16_t, 4> read() final;
    };

    class ExternalAdc : public AdcInterface {
      private:
        Mcp3204Dma m_mcp3204;

      public:
        ExternalAdc(const Config::ExternalAdc &config);
        virtual std::array<uint16_t, 4> read() final;
    };

    Config m_config;
    std::unique_ptr<AdcInterface> m_adc;
    std::map<Id, Pad> m_pads;

  private:
    void updateRollCounter(Utils::InputState &input_state);
    void updateDigitalInputState(Utils::InputState &input_state, const std::map<Id, uint16_t> &raw_values);
    void updateAnalogInputState(Utils::InputState &input_state, const std::map<Id, uint16_t> &raw_values);
    std::map<Id, uint16_t> readInputs();

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);

    void setDebounceDelay(const uint16_t delay);
    void setTriggerThresholds(const Config::Thresholds &thresholds);

    void setDoubleTriggerMode(const Config::DoubleTriggerMode mode);
    void setDoubleThresholds(const Config::Thresholds &thresholds);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DRUM_H_