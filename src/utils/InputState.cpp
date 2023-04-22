#include "utils/InputState.h"

#include <sstream>

namespace Doncon::Utils {

InputState::InputState()
    : drum({false, false, false, false}), m_xinput_report({0x00, sizeof(xinput_report_t), 0, 0, 0, 0, 0, 0, 0, 0, {}}) {
}

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
    m_xinput_report.buttons1 = 0                                //
                               | (false ? (1 << 0) : 0)         // Dpad Up
                               | (drum.don_left ? (1 << 1) : 0) // Dpad Down
                               | (drum.ka_left ? (1 << 2) : 0)  // Dpad Left
                               | (false ? (1 << 3) : 0)         // Dpad Right
                               | (false ? (1 << 4) : 0)         // Start
                               | (false ? (1 << 5) : 0)         // Select
                               | (false ? (1 << 6) : 0)         // L3
                               | (false ? (1 << 7) : 0);        // R3

    m_xinput_report.buttons2 = 0                                 //
                               | (false ? (1 << 0) : 0)          // L1
                               | (false ? (1 << 1) : 0)          // R1
                               | (false ? (1 << 2) : 0)          // Guide
                               | (drum.don_right ? (1 << 4) : 0) // A
                               | (drum.ka_right ? (1 << 5) : 0)  // B
                               | (false ? (1 << 6) : 0)          // X
                               | (false ? (1 << 7) : 0);         // Y

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

    out << "Ka Left: " << drum.ka_left << " "     //
        << "Don Left: " << drum.don_left << " "   //
        << "Don Right: " << drum.don_right << " " //
        << "Ka Right: " << drum.ka_right << " "   //
        << "\r";

    m_debug_report = out.str();

    return {(uint8_t *)m_debug_report.c_str(), static_cast<uint16_t>(m_debug_report.size() + 1)};
}

} // namespace Doncon::Utils
