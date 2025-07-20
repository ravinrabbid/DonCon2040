#ifndef _UTILS_MENU_H_
#define _UTILS_MENU_H_

#include "utils/InputState.h"
#include "utils/SettingsStore.h"

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace Doncon::Utils {

class Menu {
  public:
    enum class Page {
        Main,

        DeviceMode,
        Drum,
        Led,
        Reset,
        Bootsel,

        DrumDebounceDelay,
        DrumTriggerThresholdKaLeft,
        DrumTriggerThresholdDonLeft,
        DrumTriggerThresholdDonRight,
        DrumTriggerThresholdKaRight,

        LedBrightness,
        LedEnablePlayerColor,

        BootselMsg,
    };

    struct State {
        Page page;
        uint16_t selected_value;
        uint16_t original_value;
    };

    struct Descriptor {
        enum class Type {
            Menu,
            Selection,
            Value,
            Toggle,
            RebootInfo,
        };

        enum class Action {
            None,
            GotoParent,

            GotoPageDeviceMode,
            GotoPageDrum,
            GotoPageLed,
            GotoPageReset,
            GotoPageBootsel,

            GotoPageDrumDebounceDelay,
            GotoPageDrumTriggerThresholdKaLeft,
            GotoPageDrumTriggerThresholdDonLeft,
            GotoPageDrumTriggerThresholdDonRight,
            GotoPageDrumTriggerThresholdKaRight,

            GotoPageLedBrightness,
            GotoPageLedEnablePlayerColor,

            SetUsbMode,

            SetDrumDebounceDelay,
            SetDrumTriggerThresholdKaLeft,
            SetDrumTriggerThresholdDonLeft,
            SetDrumTriggerThresholdDonRight,
            SetDrumTriggerThresholdKaRight,

            SetLedBrightness,
            SetLedEnablePlayerColor,

            DoReset,
            DoRebootToBootsel,
        };

        Type type;
        std::string name;
        std::vector<std::pair<std::string, Action>> items;
        uint16_t max_value;
    };

    const static std::map<Page, const Descriptor> descriptors;

  private:
    std::shared_ptr<SettingsStore> m_store;
    bool m_active;
    std::stack<State> m_state_stack;

    uint16_t getCurrentValue(Page page);
    void gotoPage(Page page);
    void gotoParent(bool do_restore);

    void performAction(Descriptor::Action action, uint16_t value);

  public:
    Menu(std::shared_ptr<SettingsStore> settings_store);

    void activate();
    void update(const InputState::Controller &controller_state);
    bool active();
    State getState();
};
} // namespace Doncon::Utils

#endif // _UTILS_MENU_H_