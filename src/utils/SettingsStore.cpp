#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"

#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"

namespace Doncon::Utils {

static uint8_t read_byte(uint32_t offset) { return *(reinterpret_cast<uint8_t *>(XIP_BASE + offset)); }

SettingsStore::SettingsStore()
    : m_store_cache({.in_use = m_magic_byte,
                     .usb_mode = Config::Default::usb_mode,
                     .trigger_thresholds = Config::Default::drum_config.trigger_thresholds,
                     .led_brightness = Config::Default::led_config.brightness,
                     .led_enable_player_color = Config::Default::led_config.enable_player_color,
                     .debounce_delay = Config::Default::drum_config.debounce_delay_ms,
                     .double_trigger_mode = Config::Default::drum_config.double_trigger_mode,
                     .double_trigger_thresholds = Config::Default::drum_config.double_trigger_thresholds,
                     ._padding = {}}) {
    uint32_t current_page = m_flash_offset + m_flash_size - m_store_size;
    bool found_valid = false;
    for (size_t i = 0; i < m_store_pages; ++i) {
        if (read_byte(current_page) == m_magic_byte) {
            found_valid = true;
            break;
        } else {
            current_page -= m_store_size;
        }
    }

    if (found_valid) {
        m_store_cache = *(reinterpret_cast<Storecache *>(XIP_BASE + current_page));
        m_dirty = false;
    }
}

void SettingsStore::setUsbMode(const usb_mode_t mode) {
    if (mode != m_store_cache.usb_mode) {
        m_store_cache.usb_mode = mode;
        m_dirty = true;

        scheduleReboot();
    }
}

usb_mode_t SettingsStore::getUsbMode() { return m_store_cache.usb_mode; }

void SettingsStore::setTriggerThresholds(const Peripherals::Drum::Config::Thresholds &thresholds) {
    if (m_store_cache.trigger_thresholds.don_left != thresholds.don_left ||
        m_store_cache.trigger_thresholds.don_right != thresholds.don_right ||
        m_store_cache.trigger_thresholds.ka_left != thresholds.ka_left ||
        m_store_cache.trigger_thresholds.ka_right != thresholds.ka_right) {

        m_store_cache.trigger_thresholds = thresholds;
        m_dirty = true;
    }
}
Peripherals::Drum::Config::Thresholds SettingsStore::getTriggerThresholds() { return m_store_cache.trigger_thresholds; }

void SettingsStore::setDoubleTriggerMode(const Peripherals::Drum::Config::DoubleTriggerMode &mode) {
    if (m_store_cache.double_trigger_mode != mode) {
        m_store_cache.double_trigger_mode = mode;
        m_dirty = true;
    }
}
Peripherals::Drum::Config::DoubleTriggerMode SettingsStore::getDoubleTriggerMode() {
    return m_store_cache.double_trigger_mode;
}

void SettingsStore::setDoubleTriggerThresholds(const Peripherals::Drum::Config::Thresholds &thresholds) {
    if (m_store_cache.double_trigger_thresholds.don_left != thresholds.don_left ||
        m_store_cache.double_trigger_thresholds.don_right != thresholds.don_right ||
        m_store_cache.double_trigger_thresholds.ka_left != thresholds.ka_left ||
        m_store_cache.double_trigger_thresholds.ka_right != thresholds.ka_right) {

        m_store_cache.double_trigger_thresholds = thresholds;
        m_dirty = true;
    }
}
Peripherals::Drum::Config::Thresholds SettingsStore::getDoubleTriggerThresholds() {
    return m_store_cache.double_trigger_thresholds;
}

void SettingsStore::setLedBrightness(const uint8_t brightness) {
    if (m_store_cache.led_brightness != brightness) {
        m_store_cache.led_brightness = brightness;
        m_dirty = true;
    }
}
uint8_t SettingsStore::getLedBrightness() { return m_store_cache.led_brightness; }

void SettingsStore::setLedEnablePlayerColor(const bool do_enable) {
    if (m_store_cache.led_enable_player_color != do_enable) {
        m_store_cache.led_enable_player_color = do_enable;
        m_dirty = true;
    }
}
bool SettingsStore::getLedEnablePlayerColor() { return m_store_cache.led_enable_player_color; }

void SettingsStore::setDebounceDelay(const uint16_t delay) {
    if (m_store_cache.debounce_delay != delay) {
        m_store_cache.debounce_delay = delay;
        m_dirty = true;
    }
}
uint16_t SettingsStore::getDebounceDelay() { return m_store_cache.debounce_delay; }

void SettingsStore::store() {
    if (m_dirty) {
        multicore_lockout_start_blocking();
        uint32_t interrupts = save_and_disable_interrupts();

        uint32_t current_page = m_flash_offset;
        bool do_erase = true;
        for (size_t i = 0; i < m_store_pages; ++i) {
            if (read_byte(current_page) == 0xFF) {
                do_erase = false;
                break;
            } else {
                current_page += m_store_size;
            }
        }

        if (do_erase) {
            flash_range_erase(m_flash_offset, m_flash_size);
            current_page = m_flash_offset;
        }

        flash_range_program(current_page, reinterpret_cast<uint8_t *>(&m_store_cache), sizeof(m_store_cache));

        m_dirty = false;

        restore_interrupts_from_disabled(interrupts);
        multicore_lockout_end_blocking();
    }

    switch (m_scheduled_reboot) {
    case RebootType::Normal:
        watchdog_reboot(0, 0, 1);
        break;
    case RebootType::Bootsel:
        sleep_ms(100);
        reset_usb_boot(0, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
        break;
    case RebootType::None:
        break;
    }
}

void SettingsStore::reset() {
    multicore_lockout_start_blocking();
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(m_flash_offset, m_flash_size);

    restore_interrupts(interrupts);
    multicore_lockout_end_blocking();

    m_dirty = false;

    scheduleReboot();
    store();
}

void SettingsStore::scheduleReboot(const bool bootsel) {
    if (m_scheduled_reboot != RebootType::Bootsel) {
        m_scheduled_reboot = (bootsel ? RebootType::Bootsel : RebootType::Normal);
    }
}

} // namespace Doncon::Utils
