#ifndef PROPRIETARY_DEVICEDRIVER_H_
#define PROPRIETARY_DEVICEDRIVER_H_

#include "utils/InputState.h"

namespace Doncon::Proprietary {

class DeviceDriver {
  protected:
    DeviceDriver() = default;

  public:
    virtual ~DeviceDriver() = default;

    DeviceDriver(const DeviceDriver &) = default;
    DeviceDriver(DeviceDriver &&) = default;
    DeviceDriver &operator=(const DeviceDriver &) = default;
    DeviceDriver &operator=(DeviceDriver &&) = default;

    virtual void setInputState(const Utils::InputState &state) = 0;
};

} // namespace Doncon::Proprietary

#endif // PROPRIETARY_DEVICEDRIVER_H_