#include "proprietary/device/WiimoteExtensionDriver.h"

#include "utils/WiiCryptoProvider.h"

#include "pico/i2c_slave.h"
#include "pico/time.h"

namespace {

constexpr uint8_t WII_EXTENSION_I2C_ADDR = 0x52;
constexpr uint8_t WII_EXTENSION_ENCRYPTION_MODE_ENCRYPTED = 0xAA;

constexpr absolute_time_t I2C_IDLE_TIMEOUT_MS = 1000;

struct __attribute((packed, aligned(1))) Registers {
    std::array<uint8_t, 5> report_static = {0xA0, 0x20, 0x50, 0x10, 0xFF};
    volatile uint8_t report_buttons = 0xFF;
    std::array<uint8_t, 15> report_padding = {};

    std::array<uint8_t, 11> _unused_0xFF_1 = {};
    std::array<uint8_t, 32> _unused_0x00_1 = {};

    std::array<uint8_t, 16> encryption_data = {};

    std::array<uint8_t, 160> _unused_0xFF_2 = {};

    uint8_t encryption_mode = 0x55;

    std::array<uint8_t, 9> _unused_0xFF_3 = {};

    std::array<uint8_t, 6> extension_id = {0x00, 0x00, 0xA4, 0x20, 0x01, 0x11};

    constexpr Registers() {
        _unused_0xFF_1.fill(0xFF);
        encryption_data.fill(0xFF);
        _unused_0xFF_2.fill(0xFF);
        _unused_0xFF_3.fill(0xFF);
    }

    std::span<uint8_t> raw() { return {reinterpret_cast<uint8_t *>(this), sizeof(Registers)}; };
};
static_assert(sizeof(Registers) == UINT8_MAX + 1);

Registers registers{};

// This runs within an ISR!
void __time_critical_func(i2c_slave_handler)(i2c_inst_t *i2c, i2c_slave_event_t event) {
    static absolute_time_t last_event = get_absolute_time();

    static bool key_dirty = true;
    static Doncon::Utils::WiiCryptoProvider crypto;

    static bool address_written = false;
    static uint8_t address = 0x00;

    const auto now = get_absolute_time();
    const auto elapsed = absolute_time_diff_us(last_event, now);
    if (us_to_ms(elapsed) > I2C_IDLE_TIMEOUT_MS) {
        key_dirty = true;
        address_written = false;
    }
    last_event = now;

    switch (event) {
    case I2C_SLAVE_RECEIVE:
        if (!address_written) {
            address = i2c_read_byte_raw(i2c);
            address_written = true;
        } else {
            auto byte = i2c_read_byte_raw(i2c);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            registers.raw()[address] = byte;

            if (address >= offsetof(Registers, encryption_data) &&
                address < offsetof(Registers, encryption_data) + sizeof(Registers::encryption_data)) {
                key_dirty = true;
            }

            address++;
        }
        break;
    case I2C_SLAVE_REQUEST: {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        uint8_t byte = registers.raw()[address];

        if (registers.encryption_mode == WII_EXTENSION_ENCRYPTION_MODE_ENCRYPTED) {
            if (key_dirty) {
                key_dirty = !crypto.setKey(registers.encryption_data);
            }

            if (!key_dirty) {
                byte = crypto.encrypt(byte, address);
            }
        }

        i2c_write_byte_raw(i2c, byte);
        address++;
    } break;
    case I2C_SLAVE_FINISH:
        address_written = false;
        break;
    }
}

} // namespace

namespace Doncon::Proprietary {

WiimoteExtensionDriver::WiimoteExtensionDriver(const WiimoteExtensionDriver::Config &config) {
    Utils::I2c::initGpio(config);
    i2c_init(config.block, config.speed_hz);
    i2c_slave_init(config.block, WII_EXTENSION_I2C_ADDR, &i2c_slave_handler);
}

void WiimoteExtensionDriver::setInputState(const Utils::InputState &state) {
    const auto &drum = state.drum;

    // one byte writes/reads are atomic on cortex-m
    registers.report_buttons = ~(0                                           //
                                 | (drum.don_left.triggered ? (1 << 6) : 0)  //
                                 | (drum.ka_left.triggered ? (1 << 5) : 0)   //
                                 | (drum.don_right.triggered ? (1 << 4) : 0) //
                                 | (drum.ka_right.triggered ? (1 << 3) : 0));
}

} // namespace Doncon::Proprietary