#include "peripherals/Display.h"

#include "hardware/gpio.h"
#include "pico/time.h"

#include "bitmaps/MenuScreens.h"

#include <list>
#include <numeric>
#include <string>

namespace Doncon::Peripherals {

namespace {

std::string modeToString(usb_mode_t mode) {
    switch (mode) {
    case USB_MODE_SWITCH_TATACON:
        return "Switch Tatacon";
    case USB_MODE_SWITCH_HORIPAD:
        return "Switch Horipad";
    case USB_MODE_DUALSHOCK3:
        return "Dualshock 3";
    case USB_MODE_PS4_TATACON:
        return "PS4 Tatacon";
    case USB_MODE_DUALSHOCK4:
        return "Dualshock 4";
    case USB_MODE_KEYBOARD_P1:
        return "Keyboard P1";
    case USB_MODE_KEYBOARD_P2:
        return "Keyboard P2";
    case USB_MODE_XBOX360:
        return "Xbox 360";
    case USB_MODE_XBOX360_ANALOG_P1:
        return "Analog P1";
    case USB_MODE_XBOX360_ANALOG_P2:
        return "Analog P2";
    case USB_MODE_MIDI:
        return "MIDI";
    case USB_MODE_DEBUG:
        return "Debug";
    }
    return "?";
}

} // namespace

Display::Display(const Config &config) : m_config(config) {
    m_display.external_vcc = false;
    ssd1306_init(&m_display, 128, 64, m_config.i2c_address, m_config.i2c_block);
    ssd1306_clear(&m_display);
}

void Display::setInputState(const Utils::InputState &state) { m_input_state = state; }
void Display::setUsbMode(usb_mode_t mode) { m_usb_mode = mode; };
void Display::setPlayerId(uint8_t player_id) { m_player_id = player_id; };

void Display::setMenuState(const Utils::Menu::State &menu_state) { m_menu_state = menu_state; }

void Display::showIdle() { m_state = State::Idle; }
void Display::showMenu() { m_state = State::Menu; }

void Display::drawIdleScreen() {
    // Header
    const std::string mode_string = modeToString(m_usb_mode) + " mode";
    ssd1306_draw_string(&m_display, 0, 0, 1, mode_string.c_str());
    ssd1306_draw_line(&m_display, 0, 10, 128, 10);

    // Roll counter
    auto roll_str = std::to_string(m_input_state.drum.current_roll) + " Roll";
    auto prev_roll_str = "Last " + std::to_string(m_input_state.drum.previous_roll);
    ssd1306_draw_string(&m_display, (127 - (roll_str.length() * 12)) / 2, 20, 2, roll_str.c_str());
    ssd1306_draw_string(&m_display, (127 - (prev_roll_str.length() * 6)) / 2, 40, 1, prev_roll_str.c_str());

    // Player "LEDs"
    if (m_player_id != 0) {
        for (uint8_t i = 0; i < 4; ++i) {
            if ((m_player_id & (1 << i)) == 0) {
                ssd1306_draw_square(&m_display, (127) - ((4 - i) * 6), 3, 2, 2);
            } else {
                ssd1306_draw_square(&m_display, ((127) - ((4 - i) * 6)) - 1, 2, 4, 4);
            }
        }
    }

    // Menu hint
    ssd1306_draw_line(&m_display, 0, 54, 128, 54);
    ssd1306_draw_string(&m_display, 0, 56, 1, "Hold STA+SEL for Menu");
}

void Display::drawMenuScreen() {
    auto descriptor_it = Utils::Menu::descriptors.find(m_menu_state.page);
    if (descriptor_it == Utils::Menu::descriptors.end()) {
        return;
    }

    // Background
    switch (descriptor_it->second.type) {
    case Utils::Menu::Descriptor::Type::Menu:
        if (m_menu_state.page == Utils::Menu::Page::Main) {
            ssd1306_bmp_show_image(&m_display, menu_screen_top.data(), menu_screen_top.size());
        } else {
            ssd1306_bmp_show_image(&m_display, menu_screen_sub.data(), menu_screen_sub.size());
        }
        break;
    case Utils::Menu::Descriptor::Type::Value:
        ssd1306_bmp_show_image(&m_display, menu_screen_value.data(), menu_screen_value.size());
        break;
    case Utils::Menu::Descriptor::Type::Selection:
    case Utils::Menu::Descriptor::Type::Toggle:
        ssd1306_bmp_show_image(&m_display, menu_screen_sub.data(), menu_screen_sub.size());
        break;
    case Utils::Menu::Descriptor::Type::RebootInfo:
        break;
    }

    // Heading
    ssd1306_draw_string(&m_display, 0, 0, 1, descriptor_it->second.name.c_str());

    // Current Selection
    std::string selection;
    switch (descriptor_it->second.type) {
    case Utils::Menu::Descriptor::Type::Menu:
    case Utils::Menu::Descriptor::Type::Selection:
    case Utils::Menu::Descriptor::Type::RebootInfo:
        selection = descriptor_it->second.items.at(m_menu_state.selected_value).first;
        break;
    case Utils::Menu::Descriptor::Type::Value:
        selection = std::to_string(m_menu_state.selected_value);
        break;
    case Utils::Menu::Descriptor::Type::Toggle:
        selection = m_menu_state.selected_value != 0 ? "On" : "Off";
        break;
    }
    ssd1306_draw_string(&m_display, (127 - (selection.length() * 12)) / 2, 15, 2, selection.c_str());

    // Breadcrumbs
    switch (descriptor_it->second.type) {
    case Utils::Menu::Descriptor::Type::Menu:
    case Utils::Menu::Descriptor::Type::Selection: {
        auto selection_count = descriptor_it->second.items.size();
        for (size_t i = 0; i < selection_count; ++i) {
            if (i == m_menu_state.selected_value) {
                ssd1306_draw_square(&m_display, ((127) - ((selection_count - i) * 6)) - 1, 2, 4, 4);
            } else {
                ssd1306_draw_square(&m_display, (127) - ((selection_count - i) * 6), 3, 2, 2);
            }
        }
    } break;
    case Utils::Menu::Descriptor::Type::RebootInfo:
    case Utils::Menu::Descriptor::Type::Value:
    case Utils::Menu::Descriptor::Type::Toggle:
        break;
    }
}

void Display::update() {
    static const uint32_t interval_ms = 17; // Limit to ~60fps

    if (to_ms_since_boot(get_absolute_time()) - m_next_frame_time < interval_ms) {
        return;
    }
    m_next_frame_time += interval_ms;

    ssd1306_clear(&m_display);

    switch (m_state) {
    case State::Idle:
        drawIdleScreen();
        break;
    case State::Menu:
        drawMenuScreen();
        break;
    }

    ssd1306_show(&m_display);
};

} // namespace Doncon::Peripherals