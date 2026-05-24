#ifndef EXTENSIONS_WII_I2CSLAVE_H_
#define EXTENSIONS_WII_I2CSLAVE_H_

#include "utils/I2c.h"

#include <cstdint>

namespace Doncon::Extensions::Wii {

void init(const Utils::I2c::Config &config);

void setReport(uint8_t report);

} // namespace Doncon::Extensions::Wii

#endif // EXTENSIONS_WII_I2CSLAVE_H_