#ifndef _UTILS_INPUTSTATE_H_
#define _UTILS_INPUTSTATE_H_

#include "usb/device/hid/keyboard_driver.h"
#include "usb/device/hid/ps3_driver.h"
#include "usb/device/hid/ps4_driver.h"
#include "usb/device/hid/switch_driver.h"
#include "usb/device/midi_driver.h"
#include "usb/device/vendor/xinput_driver.h"
#include "usb/device_driver.h"

#include <stdint.h>
#include <string>

namespace Doncon::Utils {

struct InputState {
  public:
    struct Drum {
        struct Pad {
            bool triggered;
            uint16_t analog;
            uint16_t raw;
        };

        Pad don_left, ka_left, don_right, ka_right;
        uint16_t current_roll;
        uint16_t previous_roll;
    };

    struct Controller {
        struct DPad {
            bool up, down, left, right;
        };

        struct Buttons {
            bool north, east, south, west;
            bool l, r;
            bool start, select, home, share;
        };

        DPad dpad;
        Buttons buttons;
    };

  public:
    Drum drum;
    Controller controller;

  private:
    enum class Player {
        One,
        Two,
    };

  private:
    hid_switch_report_t m_switch_report;
    hid_ps3_report_t m_ps3_report;
    hid_ps4_report_t m_ps4_report;
    hid_nkro_keyboard_report_t m_keyboard_report;
    xinput_report_t m_xinput_report;
    midi_report_t m_midi_report;
    std::string m_debug_report;

    usb_report_t getSwitchReport();
    usb_report_t getPS3InputReport();
    usb_report_t getPS4InputReport();
    usb_report_t getKeyboardReport(Player player);
    usb_report_t getXinputBaseReport();
    usb_report_t getXinputDigitalReport();
    usb_report_t getXinputAnalogReport(Player player);
    usb_report_t getMidiReport();
    usb_report_t getDebugReport();

  public:
    InputState();

    usb_report_t getReport(usb_mode_t mode);

    void releaseAll();

    bool checkHotkey();
};

} // namespace Doncon::Utils

#endif // _UTILS_INPUTSTATE_H_