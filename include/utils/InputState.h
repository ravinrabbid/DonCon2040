#ifndef _UTILS_INPUTSTATE_H_
#define _UTILS_INPUTSTATE_H_

#include "usb/usb_driver.h"

#include <stdint.h>
#include <string>

namespace Doncon::Utils {

struct InputState {
  public:
    struct Drum {
        bool don_left, ka_left, don_right, ka_right;
    };

  public:
    Drum drum;

  private:
    std::string m_debug_report;

    usb_report_t getDebugReport();

  public:
    InputState();

    usb_report_t getReport(usb_mode_t mode);
};

} // namespace Doncon::Utils

#endif // _UTILS_INPUTSTATE_H_