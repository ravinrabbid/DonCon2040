#ifndef _UTILS_MENU_H_
#define _UTILS_MENU_H_

#include "utils/InputState.h"
#include "utils/SettingsStore.h"

#include <map>
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>

namespace Doncon::Utils {

class Menu {
  public:
    enum class Page {
        None,
        Main,
        DeviceMode,
        TriggerThreshold,
        TriggerThresholdKaLeft,
        TriggerThresholdDonLeft,
        TriggerThresholdDonRight,
        TriggerThresholdKaRight,
        TriggerThresholdScaleLevel,
        LedBrightness,
        Reset,
        Bootsel,
        BootselMsg,
    };

    struct State {
        Page page;
        uint16_t selection;
    };

    struct Descriptor {
        enum class Type {
            Root,
            Selection,
            Value,
            RebootInfo,
        };

        enum class Action {
            None,
            GotoParent,

            GotoPageDeviceMode,
            GotoPageTriggerThreshold,
            GotoPageTriggerThresholdKaLeft,
            GotoPageTriggerThresholdDonLeft,
            GotoPageTriggerThresholdDonRight,
            GotoPageTriggerThresholdKaRight,
            GotoPageTriggerThresholdScaleLevel,
            GotoPageLedBrightness,
            GotoPageReset,
            GotoPageBootsel,

            ChangeUsbModeSwitchTatacon,
            ChangeUsbModeSwitchHoripad,
            ChangeUsbModeDS3,
            ChangeUsbModePS4Tatacon,
            ChangeUsbModeDS4,
            ChangeUsbModeXbox360,
            ChangeUsbModeDebug,

            SetTriggerThresholdKaLeft,
            SetTriggerThresholdDonLeft,
            SetTriggerThresholdDonRight,
            SetTriggerThresholdKaRight,
            SetTriggerThresholdScaleLevel,
            SetLedBrightness,

            DoRebootToBootsel,
            DoReset,
        };

        Type type;
        std::string name;
        std::vector<std::pair<std::string, Action>> items;
        uint16_t max_value;
        Page parent;
    };

    const static std::map<Page, const Descriptor> descriptors;

  private:
    std::shared_ptr<SettingsStore> m_store;
    bool m_active;
    State m_state;

    uint16_t getCurrentSelection(Page page);
    void gotoPage(Page page);
    void performSelectionAction(Descriptor::Action action);
    void performValueAction(Descriptor::Action action, uint16_t value);

  public:
    Menu(std::shared_ptr<SettingsStore> settings_store);

    void activate();
    void update(const InputState::Controller &controller_state);
    bool active();
    State getState();
};
} // namespace Doncon::Utils

#endif // _UTILS_MENU_H_