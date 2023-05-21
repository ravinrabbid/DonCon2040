#include "utils/InputState.h"

#include <iomanip>
#include <sstream>

namespace Doncon::Utils {

InputState::InputState()
    : drum({{false, 0}, {false, 0}, {false, 0}, {false, 0}}),
      controller(
          {{false, false, false, false}, {false, false, false, false, false, false, false, false, false, false}}),
      m_xinput_report({0x00, sizeof(xinput_report_t), 0, 0, 0, 0, 0, 0, 0, 0, {}}) {}

usb_report_t InputState::getReport(usb_mode_t mode) {
    switch (mode) {
    // case USB_MODE_SWITCH_TATACON:
    // case USB_MODE_SWITCH_HORIPAD:
    //     return getSwitchReport();
    // case USB_MODE_DUALSHOCK3:
    //     return getPS3InputReport();
    // case USB_MODE_PS4_DIVACON:
    // case USB_MODE_DUALSHOCK4:
    //     return getPS4InputReport();
    case USB_MODE_XBOX360:
        return getXinputReport();
    case USB_MODE_DEBUG:
    default:
        return getDebugReport();
    }
}

usb_report_t InputState::getXinputReport() {
    m_xinput_report.buttons1 = 0                                                                    //
                               | (controller.dpad.up ? (1 << 0) : 0)                                // Dpad Up
                               | ((controller.dpad.down || drum.don_left.triggered) ? (1 << 1) : 0) // Dpad Down
                               | ((controller.dpad.left || drum.ka_left.triggered) ? (1 << 2) : 0)  // Dpad Left
                               | (controller.dpad.right ? (1 << 3) : 0)                             // Dpad Right
                               | (controller.buttons.start ? (1 << 4) : 0)                          // Start
                               | (controller.buttons.select ? (1 << 5) : 0)                         // Select
                               | (false ? (1 << 6) : 0)                                             // L3
                               | (false ? (1 << 7) : 0);                                            // R3

    m_xinput_report.buttons2 = 0                                                                         //
                               | (controller.buttons.l ? (1 << 0) : 0)                                   // L1
                               | (controller.buttons.r ? (1 << 1) : 0)                                   // R1
                               | (controller.buttons.home ? (1 << 2) : 0)                                // Guide
                               | ((controller.buttons.south || drum.don_right.triggered) ? (1 << 4) : 0) // A
                               | ((controller.buttons.east || drum.ka_right.triggered) ? (1 << 5) : 0)   // B
                               | (controller.buttons.west ? (1 << 6) : 0)                                // X
                               | (controller.buttons.north ? (1 << 7) : 0);                              // Y

    m_xinput_report.lt = 0;
    m_xinput_report.rt = 0;

    m_xinput_report.lx = 0;
    m_xinput_report.ly = 0;
    m_xinput_report.rx = 0;
    m_xinput_report.ry = 0;

    return {(uint8_t *)&m_xinput_report, sizeof(xinput_report_t)};
}

usb_report_t InputState::getDebugReport() {
    std::stringstream out;

    auto bar = [](uint16_t val) { return std::string(val / 512, '#'); };

    if (drum.don_left.triggered || drum.ka_left.triggered || drum.don_right.triggered || drum.ka_right.triggered) {
        out << "(" << (drum.ka_left.triggered ? "*" : " ") << "( "                                              //
            << std::setw(4) << drum.ka_left.raw << "[" << std::setw(8) << bar(drum.ka_left.raw) << "]"          //
            << "(" << (drum.don_left.triggered ? "*" : " ") << "| "                                             //
            << std::setw(4) << drum.don_left.raw << "[" << std::setw(8) << bar(drum.don_left.raw) << "]"        //
            << "|" << (drum.don_right.triggered ? "*" : " ") << ") "                                            //
            << std::setw(4) << drum.don_right.raw << "[" << std::setw(8) << bar(drum.don_right.raw) << "]"      //
            << "(" << (drum.ka_right.triggered ? "*" : " ") << "( " << std::setw(4) << drum.ka_right.raw << "[" //
            << std::setw(8) << bar(drum.ka_right.raw) << "]"                                                    //
            << "\n";
    }

    m_debug_report = out.str();

    return {(uint8_t *)m_debug_report.c_str(), static_cast<uint16_t>(m_debug_report.size() + 1)};
}

} // namespace Doncon::Utils
