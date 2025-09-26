#include "Mcp23017.h"

#include <array>

Mcp23017::Mcp23017(uint8_t address, i2c_inst *i2c) : m_i2c(i2c), m_address(address) {

    // Sequential register addresses with automatic address pointer increment
    writeRegister8(Register::IOCON, 0x00);
}

void Mcp23017::setDirection(uint16_t input_mask) { writeRegister16(Register::IODIRA, input_mask); }

void Mcp23017::setDirection(Mcp23017::Port port, uint8_t input_mask) {
    switch (port) {
    case Port::A:
        writeRegister8(Register::IODIRA, input_mask);
        break;
    case Port::B:
        writeRegister8(Register::IODIRB, input_mask);
        break;
    }
}
void Mcp23017::setDirection(uint8_t pin, Mcp23017::Direction direction) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        setDirection(pin - 8, Port::B, direction);
    } else {
        setDirection(pin, Port::A, direction);
    }
}

void Mcp23017::setDirection(uint8_t pin, Mcp23017::Port port, Mcp23017::Direction direction) {
    if (pin > 7) {
        return;
    }

    Register target_register = Register::IODIRA;

    switch (port) {
    case Port::A:
        target_register = Register::IODIRA;
        break;
    case Port::B:
        target_register = Register::IODIRB;
        break;
    }

    const uint8_t iodirs = readRegister8(target_register);

    switch (direction) {
    case Direction::IN:
        writeRegister8(target_register, iodirs | (1 << pin));
        break;
    case Direction::OUT:
        writeRegister8(target_register, iodirs & ~(1 << pin));
        break;
    }
}

void Mcp23017::setPullup(uint16_t enable_mask) { writeRegister16(Register::GPPUA, enable_mask); }

void Mcp23017::setPullup(Mcp23017::Port port, uint8_t enable_mask) {
    switch (port) {
    case Port::A:
        writeRegister8(Register::GPPUA, enable_mask);
        break;
    case Port::B:
        writeRegister8(Register::GPPUB, enable_mask);
        break;
    }
}

void Mcp23017::setPullup(uint8_t pin, bool enable) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        setPullup(pin - 8, Port::B, enable);
    } else {
        setPullup(pin, Port::A, enable);
    }
}

void Mcp23017::setPullup(uint8_t pin, Mcp23017::Port port, bool enable) {
    if (pin > 7) {
        return;
    }

    Register target_register = Register::GPPUA;

    switch (port) {
    case Port::A:
        target_register = Register::GPPUA;
        break;
    case Port::B:
        target_register = Register::GPPUB;
        break;
    }

    const uint8_t pullups = readRegister8(target_register);

    if (enable) {
        writeRegister8(target_register, pullups | (1 << pin));
    } else {
        writeRegister8(target_register, pullups & ~(1 << pin));
    }
}

void Mcp23017::setReversePolarity(uint16_t reverse_mask) { writeRegister16(Register::IPOLA, reverse_mask); }

void Mcp23017::setReversePolarity(Port port, uint8_t reverse_mask) {
    switch (port) {
    case Port::A:
        writeRegister8(Register::IPOLA, reverse_mask);
        break;
    case Port::B:
        writeRegister8(Register::IPOLB, reverse_mask);
        break;
    }
}

void Mcp23017::setReversePolarity(uint8_t pin, bool reverse) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        setReversePolarity(pin - 8, Port::B, reverse);
    } else {
        setReversePolarity(pin, Port::A, reverse);
    }
}

void Mcp23017::setReversePolarity(uint8_t pin, Port port, bool reverse) {
    if (pin > 7) {
        return;
    }

    Register target_register = Register::IPOLA;

    switch (port) {
    case Port::A:
        target_register = Register::IPOLA;
        break;
    case Port::B:
        target_register = Register::IPOLB;
        break;
    }

    const uint8_t reversed = readRegister8(target_register);

    if (reverse) {
        writeRegister8(target_register, reversed | (1 << pin));
    } else {
        writeRegister8(target_register, reversed & ~(1 << pin));
    }
}

uint16_t Mcp23017::read() { return readRegister16(Register::GPIOA); }

uint8_t Mcp23017::read(Port port) {
    switch (port) {
    case Port::A:
        return readRegister8(Register::GPIOA);
    case Port::B:
        return readRegister8(Register::GPIOB);
    }
    return 0;
}

bool Mcp23017::read(uint8_t pin) {
    if (pin > 15) {
        return false;
    }

    if (pin > 7) {
        return read(pin - 8, Port::B);
    }
    return read(pin, Port::A);
}
bool Mcp23017::read(uint8_t pin, Port port) {
    if (pin > 7) {
        return false;
    }

    switch (port) {
    case Port::A:
        return static_cast<bool>(readRegister8(Register::GPIOA) & (1 << pin));
    case Port::B:
        return static_cast<bool>(readRegister8(Register::GPIOB) & (1 << pin));
    }
    return false;
}

void Mcp23017::write(uint16_t value) { writeRegister16(Register::GPIOA, value); }

void Mcp23017::write(Port port, uint8_t value) {
    switch (port) {
    case Port::A:
        writeRegister8(Register::GPIOA, value);
        break;
    case Port::B:
        writeRegister8(Register::GPIOB, value);
        break;
    }
}

void Mcp23017::write(uint8_t pin, bool value) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        write(pin - 8, Port::B, value);
    } else {
        write(pin, Port::A, value);
    }
}

void Mcp23017::write(uint8_t pin, Port port, bool value) {
    if (pin > 7) {
        return;
    }

    Register target_register = Register::GPIOA;

    switch (port) {
    case Port::A:
        target_register = Register::GPIOA;
        break;
    case Port::B:
        target_register = Register::GPIOB;
        break;
    }

    const uint8_t gpio = readRegister8(target_register);

    if (value) {
        writeRegister8(target_register, gpio | (1 << pin));
    } else {
        writeRegister8(target_register, gpio & ~(1 << pin));
    }
}

uint8_t Mcp23017::readRegister8(Mcp23017::Register reg) {
    uint8_t result = 0;

    const auto reg_addr = static_cast<uint8_t>(reg);
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, &result, 1, false);

    return result;
}

uint16_t Mcp23017::readRegister16(Mcp23017::Register reg) {
    std::array<uint8_t, 2> result{};

    const auto reg_addr = static_cast<uint8_t>(reg);
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, result.data(), 2, false);

    return static_cast<uint16_t>(result[1]) << 8 | static_cast<uint16_t>(result[0]);
}

void Mcp23017::writeRegister8(Mcp23017::Register reg, uint8_t value) {
    const auto reg_addr = static_cast<uint8_t>(reg);
    std::array<uint8_t, 2> data = {reg_addr, value};

    i2c_write_blocking(m_i2c, m_address, data.data(), 2, false);
}

void Mcp23017::writeRegister16(Mcp23017::Register reg, uint16_t value) {
    const auto reg_addr = static_cast<uint8_t>(reg);
    std::array<uint8_t, 3> data = {reg_addr, static_cast<uint8_t>(value & 0x00FF), static_cast<uint8_t>(value >> 8)};

    i2c_write_blocking(m_i2c, m_address, data.data(), 3, false);
}