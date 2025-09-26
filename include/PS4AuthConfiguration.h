#ifndef PS4AUTHCONFIGURATION_H_
#define PS4AUTHCONFIGURATION_H_

#include "utils/PS4AuthProvider.h"

namespace Doncon::Config::PS4Auth {

const Utils::PS4AuthProvider::Config config = {.enabled = false,       // Enabled
                                               .serial = {},           // Serial
                                               .signature = {},        // Signature
                                               .key_pem = "pem()pem"}; // Pem

} // namespace Doncon::Config::PS4Auth

#endif // PS4AUTHCONFIGURATION_H_