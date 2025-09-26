#ifndef UTILS_SETTINGSSTORE_H_
#define UTILS_SETTINGSSTORE_H_

#include "peripherals/Drum.h"
#include "usb/device_driver.h"

#include "hardware/flash.h"

#include <array>

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
        uint8_t led_brightness;
        bool led_enable_player_color;
        uint16_t debounce_delay;
        Peripherals::Drum::Config::DoubleTriggerMode double_trigger_mode;
        Peripherals::Drum::Config::Thresholds double_trigger_thresholds;

        std::array<uint8_t, m_store_size - sizeof(uint8_t) - sizeof(usb_mode_t) -
                                sizeof(Peripherals::Drum::Config::Thresholds) - sizeof(uint8_t) - sizeof(bool) -
                                sizeof(uint16_t) - sizeof(Peripherals::Drum::Config::DoubleTriggerMode) -
                                sizeof(Peripherals::Drum::Config::Thresholds)>
            _padding;
    };
    static_assert(sizeof(Storecache) == m_store_size);

    enum class RebootType : uint8_t {
        None,
        Normal,
        Bootsel,
    };

    Storecache m_store_cache;
    bool m_dirty{true};
    RebootType m_scheduled_reboot{RebootType::None};

    Storecache read();

  public:
    SettingsStore();

    void setUsbMode(usb_mode_t mode);
    [[nodiscard]] usb_mode_t getUsbMode() const;

    void setTriggerThresholds(const Peripherals::Drum::Config::Thresholds &thresholds);
    [[nodiscard]] Peripherals::Drum::Config::Thresholds getTriggerThresholds() const;

    void setDoubleTriggerMode(const Peripherals::Drum::Config::DoubleTriggerMode &mode);
    [[nodiscard]] Peripherals::Drum::Config::DoubleTriggerMode getDoubleTriggerMode() const;

    void setDoubleTriggerThresholds(const Peripherals::Drum::Config::Thresholds &thresholds);
    [[nodiscard]] Peripherals::Drum::Config::Thresholds getDoubleTriggerThresholds() const;

    void setLedBrightness(uint8_t brightness);
    [[nodiscard]] uint8_t getLedBrightness() const;

    void setLedEnablePlayerColor(bool do_enable);
    [[nodiscard]] bool getLedEnablePlayerColor() const;

    void setDebounceDelay(uint16_t delay);
    [[nodiscard]] uint16_t getDebounceDelay() const;

    void scheduleReboot(bool bootsel = false);

    void store();
    void reset();
};
} // namespace Doncon::Utils

#endif // UTILS_SETTINGSSTORE_H_