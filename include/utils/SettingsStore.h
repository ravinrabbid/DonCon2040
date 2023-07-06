#ifndef _UTILS_SETTINGSSTORE_H_
#define _UTILS_SETTINGSSTORE_H_

#include "peripherals/Drum.h"
#include "usb/usb_driver.h"

#include "hardware/flash.h"

namespace Doncon::Utils {

class SettingsStore {
  private:
    const static uint32_t m_flash_size = FLASH_SECTOR_SIZE;
    const static uint32_t m_flash_offset = PICO_FLASH_SIZE_BYTES - m_flash_size;
    const static uint32_t m_store_size = FLASH_PAGE_SIZE;
    const static uint32_t m_store_pages = m_flash_size / m_store_size;
    const static uint8_t m_magic_byte = 0x39;

    struct __attribute((packed, aligned(1))) Storecache {
        uint8_t in_use;
        usb_mode_t usb_mode;
        Peripherals::Drum::Config::Thresholds trigger_thresholds;

        uint8_t _padding[m_store_size - sizeof(uint8_t) - sizeof(usb_mode_t) -
                         sizeof(Peripherals::Drum::Config::Thresholds)];
    };
    static_assert(sizeof(Storecache) == m_store_size);

    enum class RebootType {
        None,
        Normal,
        Bootsel,
    };

    Storecache m_store_cache;
    bool m_dirty;

    RebootType m_scheduled_reboot;

  private:
    Storecache read();

  public:
    SettingsStore();

    void setUsbMode(usb_mode_t mode);
    usb_mode_t getUsbMode();

    void setTriggerThresholds(Peripherals::Drum::Config::Thresholds thresholds);
    Peripherals::Drum::Config::Thresholds getTriggerThresholds();

    void scheduleReboot(bool bootsel = false);

    void store();
};
} // namespace Doncon::Utils

#endif // _UTILS_SETTINGSSTORE_H_