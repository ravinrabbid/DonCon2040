#ifndef PROPRIETARY_DEVICE_PSXDIGITALPADDRIVER_H_
#define PROPRIETARY_DEVICE_PSXDIGITALPADDRIVER_H_

#include "proprietary/DeviceDriver.h"

#include <cstdint>

namespace Doncon::Proprietary {

class PsxDigitalPadDriver : public DeviceDriver {
  public:
    struct Config {
        uint8_t base_pin;
    };

  private:
    enum class TransferState : uint8_t {
        AWAIT_ADDRESS,
        AWAIT_COMMAND,

        COMMAND_POLL_SENT_BYTE_1,
        COMMAND_POLL_SENT_BYTE_2,
    };

    static void data_handler_func(uint8_t data_in, const volatile uint8_t **data_out, void *user_data);
    static void reset_handler_func(void *user_data);

    struct State {
        volatile TransferState transfer_state;

        volatile uint8_t buttons1;
        volatile uint8_t buttons2;
    };

    State m_state = {.transfer_state = TransferState::AWAIT_ADDRESS, .buttons1 = 0xFF, .buttons2 = 0xFF};

  public:
    explicit PsxDigitalPadDriver(const Config &config);

    void setInputState(const Utils::InputState &state) final;
};

} // namespace Doncon::Proprietary

#endif // PROPRIETARY_DEVICE_PSXDIGITALPADDRIVER_H_