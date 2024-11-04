#include "utils/Menu.h"

namespace Doncon::Utils {

const std::map<Menu::Page, const Menu::Descriptor> Menu::descriptors = {
    {Menu::Page::Main,                                            //
     {Menu::Descriptor::Type::Menu,                               //
      "Settings",                                                 //
      {{"Mode", Menu::Descriptor::Action::GotoPageDeviceMode},    //
       {"Drum", Menu::Descriptor::Action::GotoPageDrum},          //
       {"Led", Menu::Descriptor::Action::GotoPageLed},            //
       {"Reset", Menu::Descriptor::Action::GotoPageReset},        //
       {"USB Flash", Menu::Descriptor::Action::GotoPageBootsel}}, //
      0}},                                                        //

    {Menu::Page::DeviceMode,                                 //
     {Menu::Descriptor::Type::Selection,                     //
      "Mode",                                                //
      {{"Swtch Tata", Menu::Descriptor::Action::SetUsbMode}, //
       {"Swtch Pro", Menu::Descriptor::Action::SetUsbMode},  //
       {"Dualshock3", Menu::Descriptor::Action::SetUsbMode}, //
       {"PS4 Tata", Menu::Descriptor::Action::SetUsbMode},   //
       {"Dualshock4", Menu::Descriptor::Action::SetUsbMode}, //
       {"Keybrd P1", Menu::Descriptor::Action::SetUsbMode},  //
       {"Keybrd P2", Menu::Descriptor::Action::SetUsbMode},  //
       {"Xbox 360", Menu::Descriptor::Action::SetUsbMode},   //
       {"Analog P1", Menu::Descriptor::Action::SetUsbMode},  //
       {"Analog P2", Menu::Descriptor::Action::SetUsbMode},  //
       {"MIDI", Menu::Descriptor::Action::SetUsbMode},       //
       {"Debug", Menu::Descriptor::Action::SetUsbMode}},     //
      0}},                                                   //

    {Menu::Page::Drum,                                                                //
     {Menu::Descriptor::Type::Menu,                                                   //
      "Drum Settings",                                                                //
      {{"Hold Time", Menu::Descriptor::Action::GotoPageDrumDebounceDelay},            //
       {"Left Ka", Menu::Descriptor::Action::GotoPageDrumTriggerThresholdKaLeft},     //
       {"Left Don", Menu::Descriptor::Action::GotoPageDrumTriggerThresholdDonLeft},   //
       {"Right Don", Menu::Descriptor::Action::GotoPageDrumTriggerThresholdDonRight}, //
       {"Right Ka", Menu::Descriptor::Action::GotoPageDrumTriggerThresholdKaRight}},  //
      0}},                                                                            //

    {Menu::Page::DrumDebounceDelay,                           //
     {Menu::Descriptor::Type::Value,                          //
      "Hit Hold Time (ms)",                                   //
      {{"", Menu::Descriptor::Action::SetDrumDebounceDelay}}, //
      UINT8_MAX}},

    {Menu::Page::DrumTriggerThresholdKaLeft,                           //
     {Menu::Descriptor::Type::Value,                                   //
      "Trg Level Left Ka",                                             //
      {{"", Menu::Descriptor::Action::SetDrumTriggerThresholdKaLeft}}, //
      4095}},

    {Menu::Page::DrumTriggerThresholdDonLeft,                           //
     {Menu::Descriptor::Type::Value,                                    //
      "Trg Level Left Don",                                             //
      {{"", Menu::Descriptor::Action::SetDrumTriggerThresholdDonLeft}}, //
      4095}},

    {Menu::Page::DrumTriggerThresholdDonRight,                           //
     {Menu::Descriptor::Type::Value,                                     //
      "Trg Level Right Don",                                             //
      {{"", Menu::Descriptor::Action::SetDrumTriggerThresholdDonRight}}, //
      4095}},

    {Menu::Page::DrumTriggerThresholdKaRight,                           //
     {Menu::Descriptor::Type::Value,                                    //
      "Trg Level Right Ka",                                             //
      {{"", Menu::Descriptor::Action::SetDrumTriggerThresholdKaRight}}, //
      4095}},

    {Menu::Page::Led,                                                           //
     {Menu::Descriptor::Type::Menu,                                             //
      "LED Settings",                                                           //
      {{"Brightness", Menu::Descriptor::Action::GotoPageLedBrightness},         //
       {"Plyr Color", Menu::Descriptor::Action::GotoPageLedEnablePlayerColor}}, //
      0}},                                                                      //

    {Menu::Page::LedBrightness,                           //
     {Menu::Descriptor::Type::Value,                      //
      "LED Brightness",                                   //
      {{"", Menu::Descriptor::Action::SetLedBrightness}}, //
      UINT8_MAX}},                                        //

    {Menu::Page::LedEnablePlayerColor,                           //
     {Menu::Descriptor::Type::Toggle,                            //
      "Player Color (PS4)",                                      //
      {{"", Menu::Descriptor::Action::SetLedEnablePlayerColor}}, //
      0}},                                                       //

    {Menu::Page::Reset,                              //
     {Menu::Descriptor::Type::Menu,                  //
      "Reset all Settings?",                         //
      {{"No", Menu::Descriptor::Action::GotoParent}, //
       {"Yes", Menu::Descriptor::Action::DoReset}},  //
      0}},                                           //

    {Menu::Page::Bootsel,                                         //
     {Menu::Descriptor::Type::Menu,                               //
      "Reboot to Flash Mode",                                     //
      {{"Reboot?", Menu::Descriptor::Action::DoRebootToBootsel}}, //
      0}},                                                        //

    {Menu::Page::BootselMsg,                         //
     {Menu::Descriptor::Type::RebootInfo,            //
      "Ready to Flash...",                           //
      {{"BOOTSEL", Menu::Descriptor::Action::None}}, //
      0}},                                           //
};

Menu::Menu(std::shared_ptr<SettingsStore> settings_store)
    : m_store(settings_store), m_active(false), m_state_stack({{Page::Main, 0, 0}}) {};

void Menu::activate() {
    m_state_stack = std::stack<State>({{Page::Main, 0, 0}});
    m_active = true;
}

static InputState::Controller checkPressed(const InputState::Controller &controller_state) {
    struct ButtonState {
        enum State {
            Idle,
            RepeatDelay,
            Repeat,
        };
        State state;
        uint32_t pressed_since;
        uint32_t last_repeat;
    };

    static const uint32_t repeat_delay = 1000;
    static const uint32_t repeat_interval = 20;

    static ButtonState state_north = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_east = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_south = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_west = {ButtonState::State::Idle, 0, 0};

    static ButtonState state_up = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_down = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_left = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_right = {ButtonState::State::Idle, 0, 0};

    InputState::Controller result{{false, false, false, false},
                                  {false, false, false, false, false, false, false, false, false, false}};

    auto handle_button = [](ButtonState &button_state, bool input_state) {
        bool result = false;
        if (input_state) {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            switch (button_state.state) {
            case ButtonState::State::Idle:
                result = true;
                button_state.state = ButtonState::State::RepeatDelay;
                button_state.pressed_since = now;
                break;
            case ButtonState::State::RepeatDelay:
                if ((now - button_state.pressed_since) > repeat_delay) {
                    result = true;
                    button_state.state = ButtonState::State::Repeat;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            case ButtonState::State::Repeat:
                if ((now - button_state.last_repeat) > repeat_interval) {
                    result = true;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            }
        } else {
            result = false;
            button_state.state = ButtonState::State::Idle;
        }

        return result;
    };

    result.buttons.north = handle_button(state_north, controller_state.buttons.north);
    result.buttons.east = handle_button(state_east, controller_state.buttons.east);
    result.buttons.south = handle_button(state_south, controller_state.buttons.south);
    result.buttons.west = handle_button(state_west, controller_state.buttons.west);

    result.dpad.up = handle_button(state_up, controller_state.dpad.up);
    result.dpad.down = handle_button(state_down, controller_state.dpad.down);
    result.dpad.left = handle_button(state_left, controller_state.dpad.left);
    result.dpad.right = handle_button(state_right, controller_state.dpad.right);

    return result;
}

uint16_t Menu::getCurrentValue(Menu::Page page) {
    switch (page) {
    case Page::DeviceMode:
        return static_cast<uint16_t>(m_store->getUsbMode());
    case Page::DrumDebounceDelay:
        return m_store->getDebounceDelay();
    case Page::DrumTriggerThresholdKaLeft:
        return m_store->getTriggerThresholds().ka_left;
    case Page::DrumTriggerThresholdDonLeft:
        return m_store->getTriggerThresholds().don_left;
    case Page::DrumTriggerThresholdDonRight:
        return m_store->getTriggerThresholds().don_right;
    case Page::DrumTriggerThresholdKaRight:
        return m_store->getTriggerThresholds().ka_right;
    case Page::LedBrightness:
        return m_store->getLedBrightness();
    case Page::LedEnablePlayerColor:
        return static_cast<uint16_t>(m_store->getLedEnablePlayerColor());
    case Page::Main:
    case Page::Drum:
    case Page::Led:
    case Page::Reset:
    case Page::Bootsel:
    case Page::BootselMsg:
        break;
    }

    return 0;
}

void Menu::gotoPage(Menu::Page page) {
    const auto current_value = getCurrentValue(page);

    m_state_stack.push({page, current_value, current_value});
}

void Menu::gotoParent(bool do_restore) {
    const auto current_state = m_state_stack.top();

    if (current_state.page == Page::Main) {
        m_active = false;
    }

    if (do_restore) {
        switch (current_state.page) {
        case Page::DeviceMode:
            m_store->setUsbMode(static_cast<usb_mode_t>(current_state.original_value));
            break;
        case Page::DrumDebounceDelay:
            m_store->setDebounceDelay(current_state.original_value);
            break;
        case Page::DrumTriggerThresholdKaLeft: {
            auto thresholds = m_store->getTriggerThresholds();

            thresholds.ka_left = current_state.original_value;
            m_store->setTriggerThresholds(thresholds);
        } break;
        case Page::DrumTriggerThresholdDonLeft: {
            auto thresholds = m_store->getTriggerThresholds();

            thresholds.don_left = current_state.original_value;
            m_store->setTriggerThresholds(thresholds);
        } break;
        case Page::DrumTriggerThresholdDonRight: {
            auto thresholds = m_store->getTriggerThresholds();

            thresholds.don_right = current_state.original_value;
            m_store->setTriggerThresholds(thresholds);
        } break;
        case Page::DrumTriggerThresholdKaRight: {
            auto thresholds = m_store->getTriggerThresholds();

            thresholds.ka_right = current_state.original_value;
            m_store->setTriggerThresholds(thresholds);
        } break;
        case Page::LedBrightness:
            m_store->setLedBrightness(current_state.original_value);
            break;
        case Page::LedEnablePlayerColor:
            m_store->setLedEnablePlayerColor(static_cast<bool>(current_state.original_value));
            break;
        case Page::Main:
        case Page::Drum:
        case Page::Led:
        case Page::Reset:
        case Page::Bootsel:
        case Page::BootselMsg:
            break;
        }
    }

    m_state_stack.pop();
}

void Menu::performAction(Descriptor::Action action, uint8_t value) {
    switch (action) {
    case Descriptor::Action::None:
        break;
    case Descriptor::Action::GotoParent:
        gotoParent(false);
        break;
    case Descriptor::Action::GotoPageDeviceMode:
        gotoPage(Page::DeviceMode);
        break;
    case Descriptor::Action::GotoPageDrum:
        gotoPage(Page::Drum);
        break;
    case Descriptor::Action::GotoPageLed:
        gotoPage(Page::Led);
        break;
    case Descriptor::Action::GotoPageReset:
        gotoPage(Page::Reset);
        break;
    case Descriptor::Action::GotoPageBootsel:
        gotoPage(Page::Bootsel);
        break;
    case Descriptor::Action::GotoPageDrumDebounceDelay:
        gotoPage(Page::DrumDebounceDelay);
        break;
    case Descriptor::Action::GotoPageDrumTriggerThresholdKaLeft:
        gotoPage(Page::DrumTriggerThresholdKaLeft);
        break;
    case Descriptor::Action::GotoPageDrumTriggerThresholdDonLeft:
        gotoPage(Page::DrumTriggerThresholdDonLeft);
        break;
    case Descriptor::Action::GotoPageDrumTriggerThresholdDonRight:
        gotoPage(Page::DrumTriggerThresholdDonRight);
        break;
    case Descriptor::Action::GotoPageDrumTriggerThresholdKaRight:
        gotoPage(Page::DrumTriggerThresholdKaRight);
        break;
    case Descriptor::Action::GotoPageLedBrightness:
        gotoPage(Page::LedBrightness);
        break;
    case Descriptor::Action::GotoPageLedEnablePlayerColor:
        gotoPage(Page::LedEnablePlayerColor);
        break;
    case Descriptor::Action::SetUsbMode:
        m_store->setUsbMode(static_cast<usb_mode_t>(value));
        break;
    case Descriptor::Action::SetDrumDebounceDelay:
        m_store->setDebounceDelay(value);
        break;
    case Descriptor::Action::SetDrumTriggerThresholdKaLeft: {
        auto thresholds = m_store->getTriggerThresholds();

        thresholds.ka_left = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetDrumTriggerThresholdDonLeft: {
        auto thresholds = m_store->getTriggerThresholds();

        thresholds.don_left = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetDrumTriggerThresholdDonRight: {
        auto thresholds = m_store->getTriggerThresholds();

        thresholds.don_right = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetDrumTriggerThresholdKaRight: {
        auto thresholds = m_store->getTriggerThresholds();

        thresholds.ka_right = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetLedBrightness:
        m_store->setLedBrightness(value);
        break;
    case Descriptor::Action::SetLedEnablePlayerColor:
        m_store->setLedEnablePlayerColor(static_cast<bool>(value));
        break;
    case Descriptor::Action::DoReset:
        m_store->reset();
        break;
    case Descriptor::Action::DoRebootToBootsel:
        m_store->scheduleReboot(true);
        gotoPage(Page::BootselMsg);
        break;
    }

    return;
}

void Menu::update(const InputState::Controller &controller_state) {
    InputState::Controller pressed = checkPressed(controller_state);
    State &current_state = m_state_stack.top();

    auto descriptor_it = descriptors.find(current_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    if (descriptor_it->second.type == Descriptor::Type::RebootInfo) {
        m_active = false;
    } else if (pressed.dpad.left) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
            break;
        case Descriptor::Type::Selection:
            if (current_state.selected_value == 0) {
                current_state.selected_value = descriptor_it->second.items.size() - 1;
            } else {
                current_state.selected_value--;
            }
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::Menu:
            if (current_state.selected_value == 0) {
                current_state.selected_value = descriptor_it->second.items.size() - 1;
            } else {
                current_state.selected_value--;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.right) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
            break;
        case Descriptor::Type::Selection:
            if (current_state.selected_value == descriptor_it->second.items.size() - 1) {
                current_state.selected_value = 0;
            } else {
                current_state.selected_value++;
            }
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::Menu:
            if (current_state.selected_value == descriptor_it->second.items.size() - 1) {
                current_state.selected_value = 0;
            } else {
                current_state.selected_value++;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.up) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selected_value < UINT8_MAX) {
                current_state.selected_value++;
                performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selected_value = !current_state.selected_value;
            performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Menu:
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.down) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selected_value > 0) {
                current_state.selected_value--;
                performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selected_value = !current_state.selected_value;
            performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Menu:
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.buttons.south) { // Back/Exit
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
        case Descriptor::Type::Selection:
            gotoParent(true);
            break;
        case Descriptor::Type::Menu:
            gotoParent(false);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.buttons.east) { // Select
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
        case Descriptor::Type::Selection:
            gotoParent(false);
            break;
        case Descriptor::Type::Menu:
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    }
}

bool Menu::active() { return m_active; }

Menu::State Menu::getState() { return m_state_stack.top(); }

} // namespace Doncon::Utils
