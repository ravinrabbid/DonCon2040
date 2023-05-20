#ifndef _UTILS_INPUTSTATE_H_
#define _UTILS_INPUTSTATE_H_

#include "usb/usb_driver.h"
#include "usb/xinput_driver.h"

#include <stdint.h>
#include <string>

namespace Doncon::Utils {

struct InputState {
  public:
    struct Drum {
        struct Pad {
            bool triggered;
            uint16_t raw;
        };

        Pad don_left, ka_left, don_right, ka_right;
    };

    struct DPad {
        bool up, down, left, right;
    };

    struct Buttons {
        bool north, east, south, west;
        bool l, r;
        bool start, select, home, share;
    };

  public:
    Drum drum;
    DPad dpad;
    Buttons buttons;

  private:
    xinput_report_t m_xinput_report;
    std::string m_debug_report;

    usb_report_t getXinputReport();
    usb_report_t getDebugReport();

  public:
    InputState();

    usb_report_t getReport(usb_mode_t mode);
};

} // namespace Doncon::Utils

#endif // _UTILS_INPUTSTATE_H_