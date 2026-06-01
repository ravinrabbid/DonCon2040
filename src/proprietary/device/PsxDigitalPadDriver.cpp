#include "proprietary/device/PsxDigitalPadDriver.h"

#include "pio_psx/psx_spi.h"

#include "hardware/pio.h"

namespace {

constexpr uint8_t ADDRESS_CONTROLLER = 0x01;
constexpr uint8_t COMMAND_POLL = 0x42;
constexpr uint8_t MODE_DIGITAL = 0x41;
constexpr uint8_t DATA_DESC_DIGITAL = 0x5A;

} // namespace

namespace Doncon::Proprietary {

void PsxDigitalPadDriver::data_handler_func(uint8_t data_in, const volatile uint8_t **data_out, void *user_data) {
    auto &state = *static_cast<State *>(user_data);

    switch (state.transfer_state) {
    case TransferState::AWAIT_ADDRESS:
        if (data_in == ADDRESS_CONTROLLER) {
            *data_out = &MODE_DIGITAL;
            state.transfer_state = TransferState::AWAIT_COMMAND;
        }
        break;
    case TransferState::AWAIT_COMMAND:
        if (data_in == COMMAND_POLL) {
            *data_out = &DATA_DESC_DIGITAL;
            state.transfer_state = TransferState::COMMAND_POLL_SENT_BYTE_1;
        }
        break;
    case TransferState::COMMAND_POLL_SENT_BYTE_1:
        *data_out = &state.buttons1;
        state.transfer_state = TransferState::COMMAND_POLL_SENT_BYTE_2;
        break;
    case TransferState::COMMAND_POLL_SENT_BYTE_2:
        *data_out = &state.buttons2;
        state.transfer_state = TransferState::AWAIT_ADDRESS;
        break;
    }
}

void PsxDigitalPadDriver::reset_handler_func(void *user_data) {
    auto &state = *static_cast<State *>(user_data);

    state.transfer_state = TransferState::AWAIT_ADDRESS;
}

PsxDigitalPadDriver::PsxDigitalPadDriver(const PsxDigitalPadDriver::Config &config) {
    psx_spi_init(pio0, config.base_pin, PsxDigitalPadDriver::data_handler_func, PsxDigitalPadDriver::reset_handler_func,
                 &m_state);
}

void PsxDigitalPadDriver::setInputState(const Utils::InputState &state) {
    const auto &drum = state.drum;
    const auto &controller = state.controller;

    m_state.buttons1 = ~(0                                                                    //
                         | (controller.buttons.select ? (1 << 0) : 0)                         // Select
                         | (false ? (1 << 1) : 0)                                             // L3
                         | (false ? (1 << 2) : 0)                                             // R3
                         | (controller.buttons.start ? (1 << 3) : 0)                          // Start
                         | (controller.dpad.up ? (1 << 4) : 0)                                // D-Pad Up
                         | (controller.dpad.right ? (1 << 5) : 0)                             // D-Pad Right
                         | (controller.dpad.down ? (1 << 6) : 0)                              // D-Pad Down
                         | ((drum.don_left.triggered || controller.dpad.left) ? (1 << 7) : 0) // D-Pad Left
    );

    m_state.buttons2 = ~(0                                                                        //
                         | (controller.buttons.share ? (1 << 0) : 0)                              // L2
                         | (controller.buttons.home ? (1 << 1) : 0)                               // R2
                         | ((drum.ka_left.triggered || controller.buttons.l) ? (1 << 2) : 0)      // L1
                         | ((drum.ka_right.triggered || controller.buttons.r) ? (1 << 3) : 0)     // R1
                         | (controller.buttons.north ? (1 << 4) : 0)                              // Triangle
                         | ((drum.don_right.triggered || controller.buttons.east) ? (1 << 5) : 0) // Circle
                         | (controller.buttons.south ? (1 << 6) : 0)                              // Cross
                         | (controller.buttons.west ? (1 << 7) : 0)                               // Square
    );
}

} // namespace Doncon::Proprietary