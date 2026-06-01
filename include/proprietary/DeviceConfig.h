#ifndef PROPRIETARY_DEVICECONFIG_H_
#define PROPRIETARY_DEVICECONFIG_H_

#include "proprietary/DeviceDriver.h"
#include "proprietary/device/PsxDigitalPadDriver.h"
#include "proprietary/device/WiimoteExtensionDriver.h"

#include <memory>
#include <variant>

namespace Doncon::Proprietary {

using DeviceConfig = std::variant<std::monostate, WiimoteExtensionDriver::Config, PsxDigitalPadDriver::Config>;

inline std::unique_ptr<DeviceDriver> createDevice(const DeviceConfig &config) {
    std::unique_ptr<DeviceDriver> result{nullptr};

    std::visit(
        [&](auto &&config) {
            using T = std::decay_t<decltype(config)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                result.reset();
            } else if constexpr (std::is_same_v<T, WiimoteExtensionDriver::Config>) {
                result = std::make_unique<WiimoteExtensionDriver>(config);
            } else if constexpr (std::is_same_v<T, PsxDigitalPadDriver::Config>) {
                result = std::make_unique<PsxDigitalPadDriver>(config);
            } else {
                static_assert(false, "Unknown proprietary device type!");
            }
        },
        config);

    return result;
}

} // namespace Doncon::Proprietary

#endif // PROPRIETARY_DEVICECONFIG_H_