#ifndef _PERIPHERALS_DRUM_H_
#define _PERIPHERALS_DRUM_H_

#include "utils/InputState.h"

#include "hardware/spi.h"

#include <mcp3204/Mcp3204.h>

#include <map>
#include <memory>
#include <stdint.h>

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

        struct AdcInputs {
            uint8_t don_left;
            uint8_t ka_left;
            uint8_t don_right;
            uint8_t ka_right;
        };

        AdcInputs adc_inputs;
        Thresholds trigger_thresholds;
        uint8_t trigger_threshold_scale_level;

        uint8_t sample_count;
        uint16_t debounce_delay_ms;

        bool use_external_adc;

        struct Spi {
            uint8_t mosi_pin;
            uint8_t miso_pin;
            uint8_t sclk_pin;
            uint8_t scsn_pin;
            spi_inst_t *block;
            uint speed_hz;
        } external_adc_spi_config;
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
        Pad(uint8_t channel);

        uint8_t getChannel() const { return channel; };
        bool getState() const { return active; };
        void setState(bool state, uint16_t debounce_delay);
    };

    class AdcInterface {
      public:
        virtual uint16_t read(uint8_t channel) = 0;
    };

    class InternalAdc : public AdcInterface {
      public:
        InternalAdc(const Config::AdcInputs &adc_inputs);
        virtual uint16_t read(uint8_t channel) final;
    };

    class ExternalAdc : public AdcInterface {
      private:
        Mcp3204 m_mcp3204;

      public:
        ExternalAdc(const Config::Spi &spi_config);
        virtual uint16_t read(uint8_t channel) final;
    };

    Config m_config;
    std::unique_ptr<AdcInterface> m_adc;
    std::map<Id, Pad> m_pads;

  private:
    std::map<Id, uint16_t> sampleInputs();

  public:
    Drum(const Config &config);

    void updateInputState(Utils::InputState &input_state);

    void setThresholds(const Config::Thresholds &thresholds);
    void setThresholdScaleLevel(const uint8_t threshold_scale_level);
};

} // namespace Doncon::Peripherals

#endif // _PERIPHERALS_DRUM_H_