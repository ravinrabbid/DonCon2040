#include "utils/InputState.h"

#include <sstream>

namespace Doncon::Utils {

InputState::InputState() : drum({false, false, false, false}) {}

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
    // case USB_MODE_XBOX360:
    //     return getXinputReport();
    case USB_MODE_DEBUG:
    default:
        return getDebugReport();
    }
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
