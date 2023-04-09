#include <cstdint>
#include <vector>
#include <cassert>
#include <iostream>
#include "gtest/gtest.h"
#include <type_traits>

#include "NES_opcodes.h"

#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <bitset>




//complains if I put in class CPU
constexpr uint16_t STACK = 0x0100;
constexpr uint8_t STACK_RESET = 0xfd;


class CPU {


public:
    friend class CPUTest;

    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t stack_pointer;
    std::array<uint8_t, 0xFFFF> memory;

    void remove_flag(CpuFlags flag){
        status &= ~static_cast<uint8_t>(flag);
        std::cout << status;
    };
    void add_flag(CpuFlags flag){
        status |= static_cast<uint8_t>(flag);
        std::cout << status;
    };

    void reset() {
        register_a = 0;
        register_x = 0;
        register_y = 0;
        stack_pointer = STACK_RESET;
        status = static_cast<uint8_t>(0b00100100);

        program_counter = mem_read_u16(0xFFFC);
    }

    void load(const std::vector<uint8_t>& program) {
        std::copy(program.begin(), program.end(), memory.begin() + 0x8000);
        mem_write_u16(0xFFFC, 0x8000);
    }

    void load_and_run(const std::vector<uint8_t>& program) {
        load(program);
        reset();
        run();
    }







private:

    uint8_t mem_read(uint16_t addr) const {
        return memory[static_cast<size_t>(addr)];
    }

    void mem_write(uint16_t addr, uint8_t data) {
        memory[static_cast<size_t>(addr)] = data;
    }

    uint16_t mem_read_u16(uint16_t pos) const {
        uint16_t lo = static_cast<uint16_t>(mem_read(pos));
        uint16_t hi = static_cast<uint16_t>(mem_read(pos + 1));
        return (hi << 8) | lo;
    }

    void mem_write_u16(uint16_t pos, uint16_t data) {
        uint8_t hi = static_cast<uint8_t>(data >> 8);
        uint8_t lo = static_cast<uint8_t>(data & 0xff);
        mem_write(pos, lo);
        mem_write(pos + 1, hi);
    }

    //|= OR
        void set_carry_flag() {
        status |= static_cast<uint8_t>(CpuFlags::Carry);
    }

    //&= bitwise AND assignement
    //~ = not
    void clear_carry_flag() {
        status &= ~static_cast<uint8_t>(CpuFlags::Carry);
    }


/*
    void clear_carry_flag() {
        status &= static_cast<uint8_t>(CpuFlags::Carry);
    }*/

    void add_to_register_a(uint8_t data) {
        uint16_t sum = static_cast<uint16_t>(register_a) + 
                        static_cast<uint16_t>(data) +
                       (status & static_cast<uint8_t>(CpuFlags::Carry) ? 1 : 0);

        if (sum > 0xff) {
            set_carry_flag();
        } else {
            clear_carry_flag();
        }

        uint8_t result = static_cast<uint8_t>(sum);

        if ((data ^ result) & (result ^ register_a) & 0x80) {
            status |= static_cast<uint8_t>(CpuFlags::Overflow);
        } else {
            status &= ~static_cast<uint8_t>(CpuFlags::Overflow);
        }

        set_register_a(result);
    }

    void adc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t value = mem_read(addr);
    add_to_register_a(value);
}

// Subtract with carry
void sbc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    //uint never overflown in C++: they wrap around
    add_to_register_a(static_cast<uint8_t>(static_cast<int8_t>(-data) - 1));
}

uint8_t stack_pop() {
    stack_pointer = static_cast<uint8_t>(stack_pointer + 1);
    return mem_read(static_cast<uint16_t>(STACK) + stack_pointer);
}

void stack_push(uint8_t data) {
    mem_write(static_cast<uint16_t>(STACK) + stack_pointer, data);
    stack_pointer = static_cast<uint8_t>(stack_pointer - 1);
}

void stack_push_u16(uint16_t data) {
    uint8_t hi = static_cast<uint8_t>(data >> 8);
    uint8_t lo = static_cast<uint8_t>(data & 0xff);
    stack_push(hi);
    stack_push(lo);
}

uint16_t stack_pop_u16() {
    uint16_t lo = static_cast<uint16_t>(stack_pop());
    uint16_t hi = static_cast<uint16_t>(stack_pop());
    return (hi << 8) | lo;
}

/**/

void asl_accumulator() {
    uint8_t data = register_a;
    if (data >> 7 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data <<= 1;
    set_register_a(data);
}

uint8_t asl(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data >> 7 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data <<= 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void lsr_accumulator() {
    uint8_t data = register_a;
    if (data & 1 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data >>= 1;
    set_register_a(data);
}

uint8_t lsr(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data & 1 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data >>= 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

uint8_t rol(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    bool old_carry = has_flag(CpuFlags::Carry);

    if (data >> 7 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data <<= 1;
    if (old_carry) {
        data |= 1;
    }
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void rol_accumulator() {
    uint8_t data = register_a;
    bool old_carry = has_flag(CpuFlags::Carry);

    if (data >> 7 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data <<= 1;
    if (old_carry) {
        data |= 1;
    }
    set_register_a(data);
}

uint8_t ror(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    bool old_carry = has_flag(CpuFlags::Carry);

    if (data & 1 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data >>= 1;
    if (old_carry) {
        data |= 0b10000000;
    }
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void ror_accumulator() {
    uint8_t data = register_a;
    bool old_carry = has_flag(CpuFlags::Carry);

    if (data & 1 == 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data >>= 1;
    if (old_carry) {
        data |= 0b10000000;
    }
    set_register_a(data);
}

/**/

uint8_t inc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data = data + 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void pla() {
    uint8_t data = stack_pop();
    set_register_a(data);
}

void dey() {
    register_y = register_y - 1;
    update_zero_and_negative_flags(register_y);
}

void dex() {
    register_x = register_x - 1;
    update_zero_and_negative_flags(register_x);
}

uint8_t dec(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data = data - 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void plp() {
    status = stack_pop();
    status &= ~static_cast<uint8_t>(CpuFlags::Break);
    status |= static_cast<uint8_t>(CpuFlags::Break2);
}

void php() {
    auto flags = status;
    flags |= static_cast<uint8_t>(CpuFlags::Break);
    flags |= static_cast<uint8_t>(CpuFlags::Break2);
    stack_push(flags);
}

void bit(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t result = register_a & data;
    if (result == 0) {
        status |= static_cast<uint8_t>(CpuFlags::Zero);
    } else {
        status &= ~static_cast<uint8_t>(CpuFlags::Zero);
    }


    if ((data & 0b10000000) > 0){
        this->add_flag(CpuFlags::Negative);
    }

    if ((data & 0b01000000) > 0){
        this->add_flag(CpuFlags::Negative);
    }

}

void compare(const AddressingMode& mode, uint8_t compare_with) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data <= compare_with) {
        this->add_flag(CpuFlags::Carry);
    } else {
        this->remove_flag(CpuFlags::Carry);
    }

    update_zero_and_negative_flags(compare_with - data);
}

void branch(bool condition) {
    if (condition) {
        int8_t jump = static_cast<int8_t>(mem_read(program_counter));
        uint16_t jump_addr = program_counter 
                            + 1 
                            + static_cast<uint16_t>(jump);

        program_counter = jump_addr;
    }
}

void run() {
    //const std::unordered_map<uint8_t, OpCode>& opcodes = OpCode::OPCODES_MAP;

    while (true) {
        uint8_t code = mem_read(program_counter);
        program_counter += 1;
        uint16_t program_counter_state = program_counter;

        const OpCode& opcode = opcodes.at(code);

        switch (code) {
            case 0xa9: case 0xa5: case 0xb5: case 0xad: case 0xbd: case 0xb9: case 0xa1: case 0xb1:
                lda(opcode.mode);
                break;

            case 0xAA:
                tax();
                break;
            case 0xe8:
                inx();
                break;
            case 0x00:
                return;

            // CLD
            case 0xd8:
                remove_flag((CpuFlags::DecimalMode));
                break;

            // CLI
            case 0x58:
                remove_flag((CpuFlags::InterruptDisable));
                break;

            // CLV
            case 0xb8:
                remove_flag((CpuFlags::Overflow));
                break;

            // CLC
            case 0x18:
                clear_carry_flag();
                break;

            // SEC
            case 0x38:
                set_carry_flag();
                break;

            // SEI
            case 0x78:
                add_flag(CpuFlags::InterruptDisable);
                break;

            // SED
            case 0xf8:
                add_flag(CpuFlags::DecimalMode);
                break;

            // PHA
            case 0x48:
                stack_push(register_a);
                break;

            // PLA
            case 0x68:
                pla();
                break;

            // PHP
            case 0x08:
                php();
                break;

            // PLP
            case 0x28:
                plp();
                break;

            // ADC
            case 0x69: case 0x65: case 0x75: case 0x6d: case 0x7d: case 0x79: case 0x61: case 0x71:
                adc(opcode.mode);
                break;

            // SBC
            case 0xe9: case 0xe5: case 0xf5: case 0xed: case 0xfd: case 0xf9: case 0xe1: case 0xf1:
                sbc(opcode.mode);
                break;

            // AND
            case 0x29: case 0x25: case 0x35: case 0x2d: case 0x3d: case 0x39: case 0x21: case 0x31:
                and_op(opcode.mode);
                break;

            // EOR
            case 0x49: case 0x45: case 0x55: case 0x4d: case 0x5d: case 0x59: case 0x41: case 0x51:
            eor(opcode.mode);
            break;

                    // ORA
        case 0x09: case 0x05: case 0x15: case 0x0d: case 0x1d: case 0x19: case 0x01: case 0x11:
            ora(opcode.mode);
            break;

        // LSR
        case 0x4a:
            lsr_accumulator();
            break;

        // LSR
        case 0x46: case 0x56: case 0x4e: case 0x5e:
            lsr(opcode.mode);
            break;

        // ASL
        case 0x0a:
            asl_accumulator();
            break;

        // ASL
        case 0x06: case 0x16: case 0x0e: case 0x1e:
            asl(opcode.mode);
            break;

        // ROL
        case 0x2a:
            rol_accumulator();
            break;

        // ROL
        case 0x26: case 0x36: case 0x2e: case 0x3e:
            rol(opcode.mode);
            break;

        // ROR
        case 0x6a:
            ror_accumulator();
            break;

        // ROR
        case 0x66: case 0x76: case 0x6e: case 0x7e:
            ror(opcode.mode);
            break;

        // INC
        case 0xe6: case 0xf6: case 0xee: case 0xfe:
            inc(opcode.mode);
            break;

        // INY
        case 0xc8:
            iny();
            break;

        // DEC
        case 0xc6: case 0xd6: case 0xce: case 0xde:
            dec(opcode.mode);
            break;

        // DEX
        case 0xca:
            dex();
            break;

        // DEY
        case 0x88:
            dey();
            break;

        // CMP
        case 0xc9: case 0xc5: case 0xd5: case 0xcd: case 0xdd: case 0xd9: case 0xc1: case 0xd1:
            compare(opcode.mode, register_a);
            break;

        // CPY
        case 0xc0: case 0xc4: case 0xcc:
            compare(opcode.mode, register_y);
            break;

        // CPX
        case 0xe0: case 0xe4: case 0xec:
            compare(opcode.mode, register_x);
            break;

        // JMP Absolute
        case 0x4c:
            {
                uint16_t mem_address = mem_read_u16(program_counter);
                program_counter = mem_address;
            }
            break;

            // JMP Indirect
            case 0x6c: {
                uint16_t mem_address = mem_read_u16(program_counter);
                uint16_t indirect_ref;

                if ((mem_address & 0x00FF) == 0x00FF) {
                    uint8_t lo = mem_read(mem_address);
                    uint8_t hi = mem_read(mem_address & 0xFF00);
                    indirect_ref = (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo);
                } else {
                    indirect_ref = mem_read_u16(mem_address);
                }

                program_counter = indirect_ref;
                break;
            }

            // JSR
            case 0x20: {
                stack_push_u16(program_counter + 2 - 1);
                uint16_t target_address = mem_read_u16(program_counter);
                program_counter = target_address;
                break;
            }

            // RTS
            case 0x60: {
                program_counter = stack_pop_u16() + 1;
                break;
            }

            // RTI
            case 0x40: {
                status = stack_pop();
                remove_flag(CpuFlags::Break);
                add_flag(CpuFlags::Break2);
                program_counter = stack_pop_u16();
                break;
            }

            // BNE
            case 0xd0: {
                branch(~has_flag(CpuFlags::Zero));
                break;
            }

            // BVS
            case 0x70: {
                branch(has_flag(CpuFlags::Overflow));
                break;
            }

            // BVC
            case 0x50: {
                branch(!has_flag(CpuFlags::Overflow));
                break;
            }

            // BPL
            case 0x10: {
                branch(!has_flag(CpuFlags::Negative));
                break;
            }

            // BMI
            case 0x30: {
                branch(has_flag(CpuFlags::Negative));
                break;
            }

            // BEQ
            case 0xf0: {
                branch(has_flag(CpuFlags::Zero));
                break;
            }

            // BCS
            case 0xb0: {
                branch(has_flag(CpuFlags::Carry));
                break;
            }

            // BCC
            case 0x90: {
                branch(!has_flag(CpuFlags::Carry));
                break;
            }

            // BIT
            case 0x24: case 0x2c: {
                bit(opcode.mode);
                break;
            }

            // STA
            case 0x85: case 0x95: case 0x8d: case 0x9d: case 0x99: case 0x81: case 0x91: {
                sta(opcode.mode);
                break;
            }

            // STX
            case 0x86: case 0x96: case 0x8e: {
                uint16_t addr = get_operand_address(opcode.mode);
                mem_write(addr, register_x);
                break;
            }

            // STY
            case 0x84: case 0x94: case 0x8c: {
                uint16_t addr = get_operand_address(opcode.mode);
                mem_write(addr, register_y);
                break;
            }

            // LDX
            case 0xa2: case 0xa6: case 0xb6: case 0xae: case 0xbe: {
                ldx(opcode.mode);
                break;
            }

            // LDY
            case 0xa0: case 0xa4: case 0xb4: case 0xac: case 0xbc: {
                ldy(opcode.mode);
                break;
            }

            // NOP
            case 0xea: {
                // do nothing
                break;
            }

            // TAY
            case 0xa8: {
                register_y = register_a;
                update_zero_and_negative_flags(register_y);
                break;
            }

            // TSX
            case 0xba: {
                register_x = stack_pointer;
                update_zero_and_negative_flags(register_x);
                break;
            }

            // TXA
            case 0x8a: {
                register_a = register_x;
                update_zero_and_negative_flags(register_a);
                break;
            }

            // TXS
            case 0x9a: {
                stack_pointer = register_x;
                break;
            }

            // TYA
            case 0x98: {
                register_a = register_y;
                update_zero_and_negative_flags(register_a);
                break;
            }

            default: {
                // TODO: handle unknown opcodes
                break;
            }
        }

        if (program_counter_state == program_counter) {
            program_counter += (opcode.len - 1);
        }
    }
}


// ...

uint16_t get_operand_address(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::Immediate:
            return program_counter;

        case AddressingMode::ZeroPage:
            return static_cast<uint16_t>(mem_read(program_counter));

        case AddressingMode::Absolute:
            return mem_read_u16(program_counter);

        case AddressingMode::ZeroPage_X: {
            uint8_t pos = mem_read(program_counter);
            uint16_t addr = (pos + register_x) ;// %256;
            return addr;
        }
        case AddressingMode::ZeroPage_Y: {
            uint8_t pos = mem_read(program_counter);
            uint16_t addr = (pos + register_y) ;// %256;
            return addr;
        }

        case AddressingMode::Absolute_X: {
            uint16_t base = mem_read_u16(program_counter);
            uint16_t addr = base + register_x;
            return addr;
        }
        case AddressingMode::Absolute_Y: {
            uint16_t base = mem_read_u16(program_counter);
            uint16_t addr = base + register_y;
            return addr;
        }

        case AddressingMode::Indirect_X: {
            uint8_t base = mem_read(program_counter);
            uint8_t ptr = (base + register_x) ;// %256;
            uint8_t lo = mem_read(ptr);
            uint8_t hi = mem_read((ptr + 1)) ;// %256);
            return (static_cast<uint16_t>(hi) << 8) | lo;
        }
        case AddressingMode::Indirect_Y: {
            uint8_t base = mem_read(program_counter);
            uint8_t lo = mem_read(base);
            uint8_t hi = mem_read((base + 1)) ;// %256);
            uint16_t deref_base = (static_cast<uint16_t>(hi) << 8) | lo;
            uint16_t deref = deref_base + register_y;
            return deref;
        }

        case AddressingMode::NoneAddressing:
        default:
            throw std::runtime_error("Unsupported addressing mode");
    }
}






    // Other functions need to be implemented accordingly
    // ...

    bool has_flag(CpuFlags flag) {
        return (status & static_cast<uint8_t>(flag)) != 0;
    }



// ...

void lda(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t value = mem_read(addr);

    register_a = value;
    update_zero_and_negative_flags(register_a);
}

void ldy(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    register_y = data;
    update_zero_and_negative_flags(register_y);
}

void ldx(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    register_x = data;
    update_zero_and_negative_flags(register_x);
}

void sta(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    mem_write(addr, register_a);
}

void set_register_a(uint8_t value) {
    register_a = value;
    update_zero_and_negative_flags(register_a);
}

void and_op(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data & register_a);
}

void eor(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data ^ register_a);
}

void ora(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data | register_a);
}

void tax() {
    register_x = register_a;
    update_zero_and_negative_flags(register_x);
}

void inx() {
    register_x = (register_x + 1) ;//% 256;
    update_zero_and_negative_flags(register_x);
}

void iny() {
    register_y = (register_y + 1); //% 256;
    update_zero_and_negative_flags(register_y);
}

void update_zero_and_negative_flags(uint8_t result) {
    if (result == 0) {
        add_flag(CpuFlags::Zero);
    } else {
        remove_flag(CpuFlags::Zero);
    }

    if (result & 0b1000'0000) {
        add_flag(CpuFlags::Negative);
    } else {
        remove_flag(CpuFlags::Negative);
    }
}




};



