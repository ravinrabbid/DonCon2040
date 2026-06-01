#ifndef PROPRIETARY_DEVICE_WIIMOTEEXTENSIONDRIVER_H_
#define PROPRIETARY_DEVICE_WIIMOTEEXTENSIONDRIVER_H_

#include "proprietary/DeviceDriver.h"
#include "utils/I2c.h"
#include "utils/InputState.h"

namespace Doncon::Proprietary {

class WiimoteExtensionDriver : public DeviceDriver {
  public:
    using Config = Utils::I2c::Config;

    explicit WiimoteExtensionDriver(const Config &config);

    void setInputState(const Utils::InputState &state) final;
};

} // namespace Doncon::Proprietary

#endif // PROPRIETARY_DEVICE_WIIMOTEEXTENSIONDRIVER_H_