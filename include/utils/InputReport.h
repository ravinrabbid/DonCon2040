#ifndef UTILS_INPUTREPORT_H_
#define UTILS_INPUTREPORT_H_

#include "utils/InputState.h"

#include "usb/device/hid/keyboard_driver.h"
#include "usb/device/hid/ps3_driver.h"
#include "usb/device/hid/ps4_driver.h"
#include "usb/device/hid/switch_driver.h"
#include "usb/device/midi_driver.h"
#include "usb/device/vendor/xinput_driver.h"
#include "usb/device_driver.h"

#include <cstdint>
#include <string>

namespace Doncon::Utils {

struct InputReport {
  private:
    enum class Player : uint8_t {
        One,
        Two,
    };

    hid_switch_report_t m_switch_report{
        .buttons = 0x00,
        .hat = 0x08,
        .lx = 0x80,
        .ly = 0x80,
        .rx = 0x80,
        .ry = 0x80,
        .vendor = 0x00,
    };
    hid_ps3_report_t m_ps3_report{
        .report_id = 0x01,
        .padding = 0x00,
        .buttons1 = 0x00,
        .buttons2 = 0x00,
        .buttons3 = 0x00,
        .padding1 = 0x00,
        .lx = 0x80,
        .ly = 0x80,
        .rx = 0x80,
        .ry = 0x80,
        .padding2 = {},
        .lt = 0x00,
        .rt = 0x00,
        .padding3 = {},
        .unknown_0x02_1 = 0x02,
        .battery = 0xEF,
        .unknown_0x12 = 0x12,
        .padding4 = {},
        .unknown = {0x12, 0xF8, 0x77, 0x00, 0x40},
        .acc_x = 0x01FF,
        .acc_z = 0x01FF,
        .acc_y = 0x01FF,
        .padding5 = 0x00,
        .unknown_0x02_2 = 0x02,
    };
    hid_ps4_report_t m_ps4_report{
        .report_id = 0x01,
        .lx = 0x80,
        .ly = 0x80,
        .rx = 0x80,
        .ry = 0x80,
        .buttons1 = 0x08,
        .buttons2 = 0x00,
        .buttons3 = 0x00,
        .lt = 0x00,
        .rt = 0x00,
        .sensor_timestamp = 0x0000,
        .sensor_temperature = 0x00,
        .gyrox = 0x0000,
        .gyroy = 0x0000,
        .gyroz = 0x0000,
        .accelx = 0x0000,
        .accely = 0x0000,
        .accelz = 0x0000,
        ._reserved1 = {},
        .battery = 0 | (1 << 4) | 11, // Cable connected and fully charged
        .peripheral = 0x01,
        ._reserved2 = 0x00,
        .touch_report_count = 0x00,
        .touch_report1 = {},
        .touch_report2 = {},
        .touch_report3 = {},
        ._reserved3 = {},
    };
    hid_nkro_keyboard_report_t m_keyboard_report{
        .keycodes = {},
    };
    xinput_report_t m_xinput_report{
        .report_id = 0x00,
        .report_size = sizeof(xinput_report_t),
        .buttons1 = 0x08,
        .buttons2 = 0x00,
        .lt = 0x00,
        .rt = 0x00,
        .lx = 0x0000,
        .ly = 0x0000,
        .rx = 0x0000,
        .ry = 0x0000,
        ._reserved = {},
    };
    midi_report_t m_midi_report{
        .status = {},
        .velocity = {},
    };
    std::string m_debug_report;

    uint8_t m_ps4_report_counter = 0;

    usb_report_t getSwitchReport(const InputState &state);
    usb_report_t getPS3Report(const InputState &state);
    usb_report_t getPS4Report(const InputState &state);
    usb_report_t getKeyboardReport(const InputState &state, Player player);
    usb_report_t getXinputBaseReport(const InputState &state);
    usb_report_t getXinputDigitalReport(const InputState &state);
    usb_report_t getXinputAnalogReport(const InputState &state, Player player);
    usb_report_t getMidiReport(const InputState &state);
    usb_report_t getDebugReport(const InputState &state);

  public:
    InputReport() = default;

    usb_report_t getReport(const InputState &state, usb_mode_t mode);
};

} // namespace Doncon::Utils

#endif // UTILS_INPUTREPORT_H_