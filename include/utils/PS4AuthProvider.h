#ifndef UTILS_PS4AUTHPROVIDER_H_
#define UTILS_PS4AUTHPROVIDER_H_

#include "mbedtls/pk.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace Doncon::Utils {

class PS4AuthProvider {
  public:
    static constexpr size_t SIGNATURE_LENGTH = 256;
    static constexpr size_t SERIAL_LENGTH = 16;

    struct Config {
        bool enabled;
        std::array<uint8_t, SERIAL_LENGTH> serial;
        std::array<uint8_t, SIGNATURE_LENGTH> signature;
        std::string key_pem;
    };

  private:
    bool m_key_valid{false};

    mbedtls_pk_context m_pk_context;

  public:
    PS4AuthProvider();
    ~PS4AuthProvider();

    std::optional<std::array<uint8_t, SIGNATURE_LENGTH>> sign(const std::array<uint8_t, SIGNATURE_LENGTH> &challenge);
};

} // namespace Doncon::Utils

#endif // UTILS_PS4AUTHPROVIDER_H_