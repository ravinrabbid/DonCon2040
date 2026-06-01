#ifndef UTILS_WIICRYPTOPROVIDER_H_
#define UTILS_WIICRYPTOPROVIDER_H_

#include <array>
#include <cstdint>
#include <span>

namespace Doncon::Utils {

class WiiCryptoProvider {
  public:
    static constexpr uint8_t KEY_DATA_LENGTH = 16;

  private:
    static constexpr uint8_t RAND_LENGTH = 10;
    static constexpr uint8_t KEY_LENGTH = KEY_DATA_LENGTH - RAND_LENGTH;

    std::array<uint8_t, 8> forward_table = {};
    std::array<uint8_t, 8> sbox_indices = {};

  public:
    bool setKey(std::span<uint8_t, KEY_DATA_LENGTH> key_data);

    uint8_t encrypt(uint8_t data, uint8_t address);
    uint8_t decrypt(uint8_t data, uint8_t address);
};

} // namespace Doncon::Utils

#endif // UTILS_WIICRYPTOPROVIDER_H_