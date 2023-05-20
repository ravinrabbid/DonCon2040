#ifndef _MCP23017_MCP23017_H_
#define _MCP23017_MCP23017_H_

#include "hardware/gpio.h"
#include "hardware/i2c.h"

class Mcp23017 {
  public:
    // Registers in IOCON.BANK = 0 mode
    enum class Register {
        IODIRA = 0x00,
        IODIRB = 0x01,
        IPOLA = 0x02,
        IPOLB = 0x03,
        GPINTENA = 0x04,
        GPINTENB = 0x05,
        DEFVALA = 0x06,
        DEFVALB = 0x07,
        INTCONA = 0x08,
        INTCONB = 0x09,
        IOCON = 0x0A,
        GPPUA = 0x0C,
        GPPUB = 0x0D,
        INTFA = 0x0E,
        INTFB = 0x0F,
        INTCAPA = 0x10,
        INTCAPB = 0x11,
        GPIOA = 0x12,
        GPIOB = 0x13,
        OLATA = 0x14,
        OLATB = 0x15,
    };

    enum class Port { A, B };
    enum class Direction { IN, OUT };

  private:
    i2c_inst *m_i2c;
    uint8_t m_address;

  public:
    Mcp23017(uint8_t address, i2c_inst *i2c);

    void setDirection(uint16_t input_mask);
    void setDirection(Port port, uint8_t input_mask);
    void setDirection(uint8_t pin, Direction direction);
    void setDirection(uint8_t pin, Port port, Direction direction);

    void setPullup(uint16_t enable_mask);
    void setPullup(Port port, uint8_t enable_mask);
    void setPullup(uint8_t pin, bool enable);
    void setPullup(uint8_t pin, Port port, bool enable);

    void setReversePolarity(uint16_t reverse_mask);
    void setReversePolarity(Port port, uint8_t reverse_mask);
    void setReversePolarity(uint8_t pin, bool reverse);
    void setReversePolarity(uint8_t pin, Port port, bool reverse);

    uint16_t read();
    uint8_t read(Port port);
    bool read(uint8_t pin);
    bool read(uint8_t pin, Port port);

    void write(uint16_t value);
    void write(Port port, uint8_t value);
    void write(uint8_t pin, bool value);
    void write(uint8_t pin, Port port, bool value);

  private:
    uint8_t readRegister8(Register reg);
    uint16_t readRegister16(Register reg);
    void writeRegister8(Register reg, uint8_t value);
    void writeRegister16(Register reg, uint16_t value);
};

#endif // _MCP23017_MCP23017_H_