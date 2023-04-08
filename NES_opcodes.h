// NES_opcodes.h
//#include "NES_opcodes.h"
#include <cstdint>
#include <unordered_map>
#include <stdexcept>

enum class AddressingMode {
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect_X,
    Indirect_Y,
    NoneAddressing,
};

// Status Register (P) http://wiki.nesdev.com/w/index.php/Status_flags
//
//  7 6 5 4 3 2 1 0
//  N V _ B D I Z C
//  | |   | | | | +--- Carry Flag
//  | |   | | | +----- Zero Flag
//  | |   | | +------- Interrupt Disable
//  | |   | +--------- Decimal Mode (not used on NES)
//  | |   +----------- Break Command
//  | +--------------- Overflow Flag
//  +----------------- Negative Flag
//
enum class CpuFlags : uint8_t {
    Carry             = 0b0000'0001,
    Zero              = 0b0000'0010,
    InterruptDisable  = 0b0000'0100,
    DecimalMode       = 0b0000'1000,
    Break             = 0b0001'0000,
    Break2            = 0b0010'0000,
    Overflow          = 0b0100'0000,
    Negative          = 0b1000'0000


};

class OpCode {
public:
    uint8_t code;
    const char* mnemonic;
    uint8_t len;
    uint8_t cycles;
    AddressingMode mode;

    OpCode(uint8_t code, const char* mnemonic, uint8_t len, uint8_t cycles, AddressingMode mode)
        : code(code), mnemonic(mnemonic), len(len), cycles(cycles), mode(mode) {}
    
    
    operator uint8_t() const {
        // Assuming that the underlying type of AddressingMode is uint8_t
        return static_cast<uint8_t>(*this);
    }
};


// ...

static const inline std::unordered_map<uint8_t, OpCode> opcodes = {
    {0x00, OpCode(0x00, "BRK", 1, 7, AddressingMode::NoneAddressing)},
    {0xAA, OpCode(0xAA, "TAX", 1, 2, AddressingMode::NoneAddressing)},
    {0xE8, OpCode(0xE8, "INX", 1, 2, AddressingMode::NoneAddressing)},
    {0xA9, OpCode(0xA9, "LDA", 2, 2, AddressingMode::Immediate)},
    {0xA5, OpCode(0xA5, "LDA", 2, 3, AddressingMode::ZeroPage)},
    {0xB5, OpCode(0xB5, "LDA", 2, 4, AddressingMode::ZeroPage_X)},
    {0xAD, OpCode(0xAD, "LDA", 3, 4, AddressingMode::Absolute)},
    {0xBD, OpCode(0xBD, "LDA", 3, 4, AddressingMode::Absolute_X)},
    {0xB9, OpCode(0xB9, "LDA", 3, 4, AddressingMode::Absolute_Y)},
    {0xA1, OpCode(0xA1, "LDA", 2, 6, AddressingMode::Indirect_X)},
    {0xB1, OpCode(0xB1, "LDA", 2, 5, AddressingMode::Indirect_Y)},
    {0x85, OpCode(0x85, "STA", 2, 3, AddressingMode::ZeroPage)},
    {0x95, OpCode(0x95, "STA", 2, 4, AddressingMode::ZeroPage_X)},
    {0x8D, OpCode(0x8D, "STA", 3, 4, AddressingMode::Absolute)},
    {0x9D, OpCode(0x9D, "STA", 3, 5, AddressingMode::Absolute_X)},
    {0x99, OpCode(0x99, "STA", 3, 5, AddressingMode::Absolute_Y)},
    {0x81, OpCode(0x81, "STA", 2, 6, AddressingMode::Indirect_X)},
    {0x91, OpCode(0x91, "STA", 2, 6, AddressingMode::Indirect_Y)}
};