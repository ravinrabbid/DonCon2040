#include "utils/InputState.h"

#include <iomanip>
#include <sstream>

namespace Doncon::Utils {

usb_report_t InputState::getReport(usb_mode_t mode) {
    switch (mode) {
    case USB_MODE_SWITCH_TATACON:
    case USB_MODE_SWITCH_HORIPAD:
        return getSwitchReport();
    case USB_MODE_DUALSHOCK3:
        return getPS3InputReport();
    case USB_MODE_PS4_TATACON:
    case USB_MODE_DUALSHOCK4:
        return getPS4InputReport();
    case USB_MODE_KEYBOARD_P1:
        return getKeyboardReport(Player::One);
    case USB_MODE_KEYBOARD_P2:
        return getKeyboardReport(Player::Two);
    case USB_MODE_XBOX360:
        return getXinputDigitalReport();
    case USB_MODE_XBOX360_ANALOG_P1:
        return getXinputAnalogReport(Player::One);
    case USB_MODE_XBOX360_ANALOG_P2:
        return getXinputAnalogReport(Player::Two);
    case USB_MODE_MIDI:
        return getMidiReport();
    case USB_MODE_DEBUG:
        return getDebugReport();
    }

    return getDebugReport();
}

static uint8_t getHidHat(const InputState::Controller::DPad dpad) {
    if (dpad.up && dpad.right) {
        return 0x01;
    } else if (dpad.down && dpad.right) {
        return 0x03;
    } else if (dpad.down && dpad.left) {
        return 0x05;
    } else if (dpad.up && dpad.left) {
        return 0x07;
    } else if (dpad.up) {
        return 0x00;
    } else if (dpad.right) {
        return 0x02;
    } else if (dpad.down) {
        return 0x04;
    } else if (dpad.left) {
        return 0x06;
    }

    return 0x08;
}

usb_report_t InputState::getSwitchReport() {
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

    // Center all sticks
    m_switch_report.lx = 0x80;
    m_switch_report.ly = 0x80;
    m_switch_report.rx = 0x80;
    m_switch_report.ry = 0x80;

    return {reinterpret_cast<uint8_t *>(&m_switch_report), sizeof(hid_switch_report_t)};
}

usb_report_t InputState::getPS3InputReport() {
    memset(&m_ps3_report, 0, sizeof(m_ps3_report));

    m_ps3_report.report_id = 0x01;

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

    // Center all sticks
    m_ps3_report.lx = 0x80;
    m_ps3_report.ly = 0x80;
    m_ps3_report.rx = 0x80;
    m_ps3_report.ry = 0x80;

    m_ps3_report.lt = (drum.ka_left.triggered ? 0xff : 0);
    m_ps3_report.rt = (drum.ka_right.triggered ? 0xff : 0);

    m_ps3_report.unknown_0x02_1 = 0x02;
    m_ps3_report.battery = 0xef;
    m_ps3_report.unknown_0x12 = 0x12;

    m_ps3_report.unknown[0] = 0x12;
    m_ps3_report.unknown[1] = 0xf8;
    m_ps3_report.unknown[2] = 0x77;
    m_ps3_report.unknown[3] = 0x00;
    m_ps3_report.unknown[4] = 0x40;

    m_ps3_report.acc_x = 511;
    m_ps3_report.acc_y = 511;
    m_ps3_report.acc_z = 511;

    m_ps3_report.unknown_0x02_2 = 0x02;

    return {reinterpret_cast<uint8_t *>(&m_ps3_report), sizeof(hid_ps3_report_t)};
}

usb_report_t InputState::getPS4InputReport() {
    static uint8_t report_counter = 0;

    memset(&m_ps4_report, 0, sizeof(m_ps4_report));

    m_ps4_report.report_id = 0x01;

    // Center all sticks
    m_ps4_report.lx = 0x80;
    m_ps4_report.ly = 0x80;
    m_ps4_report.rx = 0x80;
    m_ps4_report.ry = 0x80;

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
    m_ps4_report.buttons3 = (report_counter << 2)                         //
                            | (controller.buttons.home ? (1 << 0) : 0)    // PS
                            | (controller.buttons.select ? (1 << 1) : 0); // T-Pad

    m_ps4_report.lt = (drum.ka_left.triggered ? 0xFF : 0);
    m_ps4_report.rt = (drum.ka_right.triggered ? 0xFF : 0);

    m_ps4_report.battery = 0 | (1 << 4) | 11; // Cable connected and fully charged
    m_ps4_report.peripheral = 0x01;
    m_ps4_report.touch_report_count = 0;

    // This method actually gets called more often than the report is sent,
    // so counters are not consecutive ... let's see if this turns out to
    // be a problem.
    report_counter++;
    if (report_counter > (UINT8_MAX >> 2)) {
        report_counter = 0;
    }

    return {reinterpret_cast<uint8_t *>(&m_ps4_report), sizeof(hid_ps4_report_t)};
}

usb_report_t InputState::getKeyboardReport(InputState::Player player) {
    m_keyboard_report = {.keycodes = {0}};

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

usb_report_t InputState::getXinputBaseReport() {
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

    m_xinput_report.lt = 0;
    m_xinput_report.rt = 0;

    m_xinput_report.lx = 0;
    m_xinput_report.ly = 0;
    m_xinput_report.rx = 0;
    m_xinput_report.ry = 0;

    m_xinput_report.report_size = sizeof(xinput_report_t);

    return {reinterpret_cast<uint8_t *>(&m_xinput_report), sizeof(xinput_report_t)};
}

usb_report_t InputState::getXinputDigitalReport() {
    getXinputBaseReport();

    m_xinput_report.buttons1 |= (drum.don_left.triggered ? (1 << 1) : 0)   // Dpad Down
                                | (drum.ka_left.triggered ? (1 << 2) : 0); // Dpad Left

    m_xinput_report.buttons2 |= (drum.don_right.triggered ? (1 << 4) : 0)   // A
                                | (drum.ka_right.triggered ? (1 << 5) : 0); // B

    return {reinterpret_cast<uint8_t *>(&m_xinput_report), sizeof(xinput_report_t)};
}

usb_report_t InputState::getXinputAnalogReport(InputState::Player player) {
    getXinputBaseReport();

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

usb_report_t InputState::getMidiReport() {
    struct state {
        bool last_triggered;
        bool on;
        uint16_t velocity;
    };

    static state acoustic_bass_drum = {};
    static state electric_bass_drum = {};
    static state drumsticks = {};
    static state side_stick = {};

    auto set_state = [](state &target, const Drum::Pad &new_state) {
        if (new_state.triggered && !target.last_triggered) {
            target.velocity = 0;
            target.on = false;
        } else if (!new_state.triggered && target.last_triggered) {
            target.on = true;
        } else if (!new_state.triggered && !target.last_triggered) {
            target.on = false;
        }

        if (new_state.triggered && new_state.analog > target.velocity) {
            target.velocity = new_state.analog;
        }

        target.last_triggered = new_state.triggered;
    };

    set_state(acoustic_bass_drum, drum.don_left);
    set_state(electric_bass_drum, drum.don_right);
    set_state(drumsticks, drum.ka_left);
    set_state(side_stick, drum.ka_right);

    m_midi_report.status.acoustic_bass_drum = acoustic_bass_drum.on;
    m_midi_report.status.electric_bass_drum = electric_bass_drum.on;
    m_midi_report.status.drumsticks = drumsticks.on;
    m_midi_report.status.side_stick = side_stick.on;

    auto convert_range = [](uint16_t in) {
        uint16_t out = in / 256;
        return uint8_t(out > 127 ? 127 : out);
    };

    m_midi_report.velocity.acoustic_bass_drum = convert_range(acoustic_bass_drum.velocity);
    m_midi_report.velocity.electric_bass_drum = convert_range(electric_bass_drum.velocity);
    m_midi_report.velocity.drumsticks = convert_range(drumsticks.velocity);
    m_midi_report.velocity.side_stick = convert_range(side_stick.velocity);

    return {reinterpret_cast<uint8_t *>(&m_midi_report), sizeof(midi_report_t)};
}

usb_report_t InputState::getDebugReport() {
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

void InputState::releaseAll() {
    drum = {};
    controller = {};
}

bool InputState::checkHotkey() {
    static uint32_t hold_since = 0;
    static bool hold_active = false;
    static const uint32_t hold_timeout = 2000;

    if (controller.buttons.start && controller.buttons.select) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (!hold_active) {
            hold_active = true;
            hold_since = now;
        } else if ((now - hold_since) > hold_timeout) {
            hold_active = false;
            return true;
        }
    } else {
        hold_active = false;
    }
    return false;
}

} // namespace Doncon::Utils
