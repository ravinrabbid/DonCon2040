#include "Mcp23017.h"

Mcp23017::Mcp23017(uint8_t address, i2c_inst *i2c) : m_i2c(i2c), m_address(address) {

    // Sequential register addresses with automatic address pointer increment
    writeRegister8(Register::IOCON, 0x00);
}

void Mcp23017::setDirection(uint16_t input_mask) { writeRegister16(Register::IODIRA, input_mask); }

void Mcp23017::setDirection(Mcp23017::Port port, uint8_t input_mask) {
    switch (port) {
    case Port::A:
        return writeRegister8(Register::IODIRA, input_mask);
    case Port::B:
        return writeRegister8(Register::IODIRB, input_mask);
    }
}
void Mcp23017::setDirection(uint8_t pin, Mcp23017::Direction direction) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        return setDirection(pin - 8, Port::B, direction);
    }
    return setDirection(pin, Port::A, direction);
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

    uint8_t iodirs = readRegister8(target_register);

    switch (direction) {
    case Direction::IN:
        return writeRegister8(target_register, iodirs | (1 << pin));
    case Direction::OUT:
        return writeRegister8(target_register, iodirs & ~(1 << pin));
    }
}

void Mcp23017::setPullup(uint16_t enable_mask) { writeRegister16(Register::GPPUA, enable_mask); }

void Mcp23017::setPullup(Mcp23017::Port port, uint8_t enable_mask) {
    switch (port) {
    case Port::A:
        return writeRegister8(Register::GPPUA, enable_mask);
    case Port::B:
        return writeRegister8(Register::GPPUB, enable_mask);
    }
}

void Mcp23017::setPullup(uint8_t pin, bool enable) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        return setPullup(pin - 8, Port::B, enable);
    }
    return setPullup(pin, Port::A, enable);
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

    uint8_t pullups = readRegister8(target_register);

    if (enable) {
        return writeRegister8(target_register, pullups | (1 << pin));
    } else {
        return writeRegister8(target_register, pullups & ~(1 << pin));
    }
}

void Mcp23017::setReversePolarity(uint16_t reverse_mask) { writeRegister16(Register::IPOLA, reverse_mask); }

void Mcp23017::setReversePolarity(Port port, uint8_t reverse_mask) {
    switch (port) {
    case Port::A:
        return writeRegister8(Register::IPOLA, reverse_mask);
    case Port::B:
        return writeRegister8(Register::IPOLB, reverse_mask);
    }
}

void Mcp23017::setReversePolarity(uint8_t pin, bool reverse) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        return setReversePolarity(pin - 8, Port::B, reverse);
    }
    return setReversePolarity(pin, Port::A, reverse);
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

    uint8_t reversed = readRegister8(target_register);

    if (reverse) {
        return writeRegister8(target_register, reversed | (1 << pin));
    } else {
        return writeRegister8(target_register, reversed & ~(1 << pin));
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
        return readRegister8(Register::GPIOA) & (1 << pin);
    case Port::B:
        return readRegister8(Register::GPIOB) & (1 << pin);
    }
    return false;
}

void Mcp23017::write(uint16_t value) { writeRegister16(Register::GPIOA, value); }

void Mcp23017::write(Port port, uint8_t value) {
    switch (port) {
    case Port::A:
        return writeRegister8(Register::GPIOA, value);
    case Port::B:
        return writeRegister8(Register::GPIOB, value);
    }
}

void Mcp23017::write(uint8_t pin, bool value) {
    if (pin > 15) {
        return;
    }

    if (pin > 7) {
        return write(pin - 8, Port::B, value);
    }
    return write(pin, Port::A, value);
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

    uint8_t gpio = readRegister8(target_register);

    if (value) {
        return writeRegister8(target_register, gpio | (1 << pin));
    } else {
        return writeRegister8(target_register, gpio & ~(1 << pin));
    }
}

uint8_t Mcp23017::readRegister8(Mcp23017::Register reg) {
    uint8_t result;

    uint8_t reg_addr = static_cast<uint8_t>(reg);
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, &result, 1, false);

    return result;
}

uint16_t Mcp23017::readRegister16(Mcp23017::Register reg) {
    uint8_t result[2];

    uint8_t reg_addr = static_cast<uint8_t>(reg);
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, result, 2, false);

    return static_cast<uint16_t>(result[1]) << 8 | static_cast<uint16_t>(result[0]);
}

void Mcp23017::writeRegister8(Mcp23017::Register reg, uint8_t value) {
    uint8_t reg_addr = static_cast<uint8_t>(reg);
    uint8_t data[] = {reg_addr, value};

    i2c_write_blocking(m_i2c, m_address, data, 2, false);
}

void Mcp23017::writeRegister16(Mcp23017::Register reg, uint16_t value) {
    uint8_t reg_addr = static_cast<uint8_t>(reg);
    uint8_t data[] = {reg_addr, static_cast<uint8_t>(value & 0x00FF), static_cast<uint8_t>(value >> 8)};

    i2c_write_blocking(m_i2c, m_address, data, 3, false);
}