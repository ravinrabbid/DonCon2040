#include "utils/InputReport.h"

#include <iomanip>
#include <sstream>

namespace Doncon::Utils {

namespace {

uint8_t getHidHat(const InputState::Controller::DPad dpad) {
    if (dpad.up && dpad.right) {
        return 0x01;
    }
    if (dpad.down && dpad.right) {
        return 0x03;
    }
    if (dpad.down && dpad.left) {
        return 0x05;
    }
    if (dpad.up && dpad.left) {
        return 0x07;
    }
    if (dpad.up) {
        return 0x00;
    }
    if (dpad.right) {
        return 0x02;
    }
    if (dpad.down) {
        return 0x04;
    }
    if (dpad.left) {
        return 0x06;
    }

    return 0x08;
}

} // namespace

usb_report_t InputReport::getSwitchReport(const InputState &state) {
    const auto &controller = state.controller;
    const auto &drum = state.drum;

    m_switch_report.buttons = 0                                             //
                              | (controller.buttons.west ? (1 << 0) : 0)    // Y
                              | (controller.buttons.south ? (1 << 1) : 0)   // B
                              | (controller.buttons.east ? (1 << 2) : 0)    // A
                              | (controller.buttons.north ? (1 << 3) : 0)   // X
                              | (controller.buttons.l ? (1 << 4) : 0)       // L
                              | (controller.buttons.r ? (1 << 5) : 0)       // R
                              | (drum.ka_left.triggered ? (1 << 6) : 0)     // ZL
                              | (drum.ka_right.triggered ? (1 << 7) : 0)    // ZR
                              | (controller.buttons.select ? (1 << 8) : 0)  // -
                              | (controller.buttons.start ? (1 << 9) : 0)   // +
                              | (drum.don_left.triggered ? (1 << 10) : 0)   // LS
                              | (drum.don_right.triggered ? (1 << 11) : 0)  // RS
                              | (controller.buttons.home ? (1 << 12) : 0)   // Home
                              | (controller.buttons.share ? (1 << 13) : 0); // Capture

    m_switch_report.hat = getHidHat(controller.dpad);

    return {reinterpret_cast<uint8_t *>(&m_switch_report), sizeof(hid_switch_report_t)};
}

usb_report_t InputReport::getPS3Report(const InputState &state) {
    const auto &controller = state.controller;
    const auto &drum = state.drum;

    m_ps3_report.buttons1 = 0                                             //
                            | (controller.buttons.select ? (1 << 0) : 0)  // Select
                            | (drum.don_left.triggered ? (1 << 1) : 0)    // L3
                            | (drum.don_right.triggered ? (1 << 2) : 0)   // R3
                            | (controller.buttons.start ? (1 << 3) : 0)   // Start
                            | (controller.dpad.up ? (1 << 4) : 0)         // Up
                            | (controller.dpad.right ? (1 << 5) : 0)      // Right
                            | (controller.dpad.down ? (1 << 6) : 0)       // Down
                            | (controller.dpad.left ? (1 << 7) : 0);      // Left
    m_ps3_report.buttons2 = 0 | (drum.ka_left.triggered ? (1 << 0) : 0)   // L2
                            | (drum.ka_right.triggered ? (1 << 1) : 0)    // R2
                            | (controller.buttons.l ? (1 << 2) : 0)       // L1
                            | (controller.buttons.r ? (1 << 3) : 0)       // R1
                            | (controller.buttons.north ? (1 << 4) : 0)   // Triangle
                            | (controller.buttons.east ? (1 << 5) : 0)    // Circle
                            | (controller.buttons.south ? (1 << 6) : 0)   // Cross
                            | (controller.buttons.west ? (1 << 7) : 0);   // Square
    m_ps3_report.buttons3 = 0 | (controller.buttons.home ? (1 << 0) : 0); // Home

    m_ps3_report.lt = (drum.ka_left.triggered ? 0xff : 0);
    m_ps3_report.rt = (drum.ka_right.triggered ? 0xff : 0);

    return {reinterpret_cast<uint8_t *>(&m_ps3_report), sizeof(hid_ps3_report_t)};
}

usb_report_t InputReport::getPS4Report(const InputState &state) {
    const auto &controller = state.controller;
    const auto &drum = state.drum;

    m_ps4_report.buttons1 = getHidHat(controller.dpad)                    //
                            | (controller.buttons.west ? (1 << 4) : 0)    // Square
                            | (controller.buttons.south ? (1 << 5) : 0)   // Cross
                            | (controller.buttons.east ? (1 << 6) : 0)    // Circle
                            | (controller.buttons.north ? (1 << 7) : 0);  // Triangle
    m_ps4_report.buttons2 = 0                                             //
                            | (controller.buttons.l ? (1 << 0) : 0)       // L1
                            | (controller.buttons.r ? (1 << 1) : 0)       // R1
                            | (drum.ka_left.triggered ? (1 << 2) : 0)     // L2
                            | (drum.ka_right.triggered ? (1 << 3) : 0)    // R2
                            | (controller.buttons.share ? (1 << 4) : 0)   // Share
                            | (controller.buttons.start ? (1 << 5) : 0)   // Option
                            | (drum.don_left.triggered ? (1 << 6) : 0)    // L3
                            | (drum.don_right.triggered ? (1 << 7) : 0);  // R3
    m_ps4_report.buttons3 = (m_ps4_report_counter << 2)                   //
                            | (controller.buttons.home ? (1 << 0) : 0)    // PS
                            | (controller.buttons.select ? (1 << 1) : 0); // T-Pad

    m_ps4_report.lt = (drum.ka_left.triggered ? 0xFF : 0);
    m_ps4_report.rt = (drum.ka_right.triggered ? 0xFF : 0);

    // This method actually gets called more often than the report is sent,
    // so counters are not consecutive ... let's see if this turns out to
    // be a problem.
    if (++m_ps4_report_counter > (UINT8_MAX >> 2)) {
        m_ps4_report_counter = 0;
    }

    return {reinterpret_cast<uint8_t *>(&m_ps4_report), sizeof(hid_ps4_report_t)};
}

usb_report_t InputReport::getKeyboardReport(const InputState &state, InputReport::Player player) {
    const auto &controller = state.controller;
    const auto &drum = state.drum;

    m_keyboard_report = {};

    auto set_key = [&](const bool input, const uint8_t keycode) {
        if (input) {
            m_keyboard_report.keycodes[keycode / 8] |= 1 << (keycode % 8);
        }
    };

    switch (player) {
    case Player::One: {
        set_key(drum.ka_left.triggered, HID_KEY_D);
        set_key(drum.don_left.triggered, HID_KEY_F);
        set_key(drum.don_right.triggered, HID_KEY_J);
        set_key(drum.ka_right.triggered, HID_KEY_K);
    } break;
    case Player::Two: {
        set_key(drum.ka_left.triggered, HID_KEY_C);
        set_key(drum.don_left.triggered, HID_KEY_B);
        set_key(drum.don_right.triggered, HID_KEY_N);
        set_key(drum.ka_right.triggered, HID_KEY_COMMA);
    } break;
    }

    set_key(controller.dpad.up, HID_KEY_ARROW_UP);
    set_key(controller.dpad.down, HID_KEY_ARROW_DOWN);
    set_key(controller.dpad.left, HID_KEY_ARROW_LEFT);
    set_key(controller.dpad.right, HID_KEY_ARROW_RIGHT);

    set_key(controller.buttons.north, HID_KEY_L);
    set_key(controller.buttons.east, HID_KEY_BACKSPACE);
    set_key(controller.buttons.south, HID_KEY_ENTER);
    set_key(controller.buttons.west, HID_KEY_P);

    set_key(controller.buttons.l, HID_KEY_Q);
    set_key(controller.buttons.r, HID_KEY_E);

    set_key(controller.buttons.start, HID_KEY_ESCAPE);
    set_key(controller.buttons.select, HID_KEY_TAB);
    // set_key(controller.buttons.home, );
    // set_key(controller.buttons.share, );

    return {reinterpret_cast<uint8_t *>(&m_keyboard_report), sizeof(hid_nkro_keyboard_report_t)};
}

usb_report_t InputReport::getXinputBaseReport(const InputState &state) {
    const auto &controller = state.controller;

    m_xinput_report.buttons1 = 0                                            //
                               | (controller.dpad.up ? (1 << 0) : 0)        // Dpad Up
                               | (controller.dpad.down ? (1 << 1) : 0)      // Dpad Down
                               | (controller.dpad.left ? (1 << 2) : 0)      // Dpad Left
                               | (controller.dpad.right ? (1 << 3) : 0)     // Dpad Right
                               | (controller.buttons.start ? (1 << 4) : 0)  // Start
                               | (controller.buttons.select ? (1 << 5) : 0) // Select
                               | (false ? (1 << 6) : 0)                     // L3
                               | (false ? (1 << 7) : 0);                    // R3
    m_xinput_report.buttons2 = 0                                            //
                               | (controller.buttons.l ? (1 << 0) : 0)      // L1
                               | (controller.buttons.r ? (1 << 1) : 0)      // R1
                               | (controller.buttons.home ? (1 << 2) : 0)   // Guide
                               | (controller.buttons.south ? (1 << 4) : 0)  // A
                               | (controller.buttons.east ? (1 << 5) : 0)   // B
                               | (controller.buttons.west ? (1 << 6) : 0)   // X
                               | (controller.buttons.north ? (1 << 7) : 0); // Y

    return {reinterpret_cast<uint8_t *>(&m_xinput_report), sizeof(xinput_report_t)};
}

usb_report_t InputReport::getXinputDigitalReport(const InputState &state) {
    const auto &drum = state.drum;

    getXinputBaseReport(state);

    m_xinput_report.buttons1 |= (drum.don_left.triggered ? (1 << 1) : 0)   // Dpad Down
                                | (drum.ka_left.triggered ? (1 << 2) : 0); // Dpad Left

    m_xinput_report.buttons2 |= (drum.don_right.triggered ? (1 << 4) : 0)   // A
                                | (drum.ka_right.triggered ? (1 << 5) : 0); // B

    return {reinterpret_cast<uint8_t *>(&m_xinput_report), sizeof(xinput_report_t)};
}

usb_report_t InputReport::getXinputAnalogReport(const InputState &state, InputReport::Player player) {
    const auto &drum = state.drum;

    getXinputBaseReport(state);

    int16_t x = 0;
    int16_t y = 0;

    auto map_to_axis = [](uint16_t raw) { return (int16_t)(raw >> 1); };

    if (drum.ka_left.analog > drum.don_left.analog) {
        x = (int16_t)-map_to_axis(drum.ka_left.analog);
    } else {
        x = map_to_axis(drum.don_left.analog);
    }

    if (drum.ka_right.analog > drum.don_right.analog) {
        y = map_to_axis(drum.ka_right.analog);
    } else {
        y = (int16_t)-map_to_axis(drum.don_right.analog);
    }

    switch (player) {
    case Player::One:
        m_xinput_report.lx = x;
        m_xinput_report.ly = y;
        break;
    case Player::Two:
        m_xinput_report.rx = x;
        m_xinput_report.ry = y;
        break;
    }

    return {reinterpret_cast<uint8_t *>(&m_xinput_report), sizeof(xinput_report_t)};
}

usb_report_t InputReport::getMidiReport(const InputState &state) {
    const auto &drum = state.drum;

    m_midi_report.status.acoustic_bass_drum = drum.don_left.triggered;
    m_midi_report.status.electric_bass_drum = drum.don_right.triggered;
    m_midi_report.status.drumsticks = drum.ka_left.triggered;
    m_midi_report.status.side_stick = drum.ka_right.triggered;

    auto convert_range = [](uint16_t in) {
        const uint16_t out = in / 256;
        return uint8_t(out > 127 ? 127 : out);
    };

    m_midi_report.velocity.acoustic_bass_drum = convert_range(drum.don_left.analog);
    m_midi_report.velocity.electric_bass_drum = convert_range(drum.don_right.analog);
    m_midi_report.velocity.drumsticks = convert_range(drum.ka_left.analog);
    m_midi_report.velocity.side_stick = convert_range(drum.ka_right.analog);

    return {reinterpret_cast<uint8_t *>(&m_midi_report), sizeof(midi_report_t)};
}

usb_report_t InputReport::getDebugReport(const InputState &state) {
    const auto &drum = state.drum;

    std::stringstream out;

    auto bar = [](uint16_t val) { return std::string(val / 511, '#'); };

    if (drum.don_left.triggered || drum.ka_left.triggered || drum.don_right.triggered || drum.ka_right.triggered) {
        out << "(" << (drum.ka_left.triggered ? "*" : " ") << "( "                                         //
            << std::setw(4) << drum.ka_left.raw << "[" << std::setw(8) << bar(drum.ka_left.raw) << "]"     //
            << "(" << (drum.don_left.triggered ? "*" : " ") << "| "                                        //
            << std::setw(4) << drum.don_left.raw << "[" << std::setw(8) << bar(drum.don_left.raw) << "]"   //
            << "|" << (drum.don_right.triggered ? "*" : " ") << ") "                                       //
            << std::setw(4) << drum.don_right.raw << "[" << std::setw(8) << bar(drum.don_right.raw) << "]" //
            << ")" << (drum.ka_right.triggered ? "*" : " ") << ") "                                        //
            << std::setw(4) << drum.ka_right.raw << "[" << std::setw(8) << bar(drum.ka_right.raw) << "]"   //
            << "\n";
    }

    m_debug_report = out.str();

    return {reinterpret_cast<uint8_t *>(m_debug_report.data()), static_cast<uint16_t>(m_debug_report.size() + 1)};
}

usb_report_t InputReport::getReport(const InputState &state, usb_mode_t mode) {
    switch (mode) {
    case USB_MODE_SWITCH_TATACON:
    case USB_MODE_SWITCH_HORIPAD:
        return getSwitchReport(state);
    case USB_MODE_DUALSHOCK3:
        return getPS3Report(state);
    case USB_MODE_PS4_TATACON:
    case USB_MODE_DUALSHOCK4:
        return getPS4Report(state);
    case USB_MODE_KEYBOARD_P1:
        return getKeyboardReport(state, Player::One);
    case USB_MODE_KEYBOARD_P2:
        return getKeyboardReport(state, Player::Two);
    case USB_MODE_XBOX360:
        return getXinputDigitalReport(state);
    case USB_MODE_XBOX360_ANALOG_P1:
        return getXinputAnalogReport(state, Player::One);
    case USB_MODE_XBOX360_ANALOG_P2:
        return getXinputAnalogReport(state, Player::Two);
    case USB_MODE_MIDI:
        return getMidiReport(state);
    case USB_MODE_DEBUG:
        return getDebugReport(state);
    }

    return getDebugReport(state);
}

} // namespace Doncon::Utils
