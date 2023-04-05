#include <cstdint>
#include <vector>
#include <cassert>
#include <iostream>
#include "gtest/gtest.h"
#include <type_traits>

#include <bitset>


//complains if I put in class CPU
constexpr uint16_t STACK = 0x0100;
constexpr uint8_t STACK_RESET = 0xfd;

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

    /*// Define bitwise operators for CpuFlags
    constexpr CpuFlags operator|(CpuFlags lhs, CpuFlags rhs) {
        using T = std::underlying_type_t<CpuFlags>;
        return static_cast<CpuFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    constexpr CpuFlags operator&(CpuFlags lhs, CpuFlags rhs) {
        using T = std::underlying_type_t<CpuFlags>;
        return static_cast<CpuFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    constexpr CpuFlags& operator|=(CpuFlags& lhs, CpuFlags rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    constexpr CpuFlags& operator&=(CpuFlags& lhs, CpuFlags rhs) {
        lhs = lhs & rhs;
        return lhs;
    }*/



class CPU {
public:



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


    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t stack_pointer;
    std::array<uint8_t, 0xFFFF> memory;

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





    void interpret(const std::vector<uint8_t>& program) {
        program_counter = 0;

        while (true) {
            uint8_t opcode = program[program_counter];
            program_counter++;

            switch (opcode) {
                case 0xA9: {
                    uint8_t param = program[program_counter];
                    program_counter++;

                    lda(param);
                    break;
                }

                case 0x00:
                    return;

                case 0xAA:
                    tax();
                    break;

                case 0xE8:
                    inx();
                    break;

                default:
                    assert(false && "Unimplemented opcode");
            }
        }
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
    flags |= ~static_cast<uint8_t>(CpuFlags::Break);
    flags |= static_cast<uint8_t>(CpuFlags::Break2);
    stack_push(flags);
}

void bit(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t result = register_a & data;
    if (result == 0) {
        status |= ~static_cast<uint8_t>(CpuFlags::Zero);
    } else {
        status &= ~static_cast<uint8_t>(CpuFlags::Zero);
    }


/*-----------------------------------------------*/
    status.set(CpuFlags::Negative, data & 0b10000000 > 0);
    status.set(CpuFlags::Overflow, data & 0b01000000 > 0);
}

void compare(const AddressingMode& mode, uint8_t compare_with) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data <= compare_with) {
        status.insert(CpuFlags::Carry);
    } else {
        status.remove(CpuFlags::Carry);
    }

    update_zero_and_negative_flags(compare_with - data);
}

void branch(bool condition) {
    if (condition) {
        int8_t jump = static_cast<int8_t>(mem_read(program_counter));
        uint16_t jump_addr = program_counter + 1 + static_cast<uint16_t>(jump);

        program_counter = jump_addr;
    }
}


    // Other functions need to be implemented accordingly
    // ...

    bool has_flag(CpuFlags flag) const {
        return static_cast<bool>(status & flag);
    }

    void set_register_a(uint8_t value) {
        register_a = value;
        update_zero_and_negative_flags(register_a);
    }

    //load into register A
    void lda(uint8_t value) {
        //loads utf-8 value into "A" or accumulator register
        register_a = value;
        update_zero_and_negative_flags(register_a);
    }

    void tax() {
        //loads value in A into X
        register_x = register_a;
        update_zero_and_negative_flags(register_x);
    }

    void inx() {
        register_x = static_cast<uint8_t>(register_x + 1);
        update_zero_and_negative_flags(register_x);
    }

    
    void update_zero_and_negative_flags(uint8_t result) {
        if (result == 0) {
            //sets the ZERO flag to zero
            // |= denotes OR bitwise operation
            status |= 0b0000'0010;
        } else {
            //sets it to 1
            //&= denotes AND bitwise operation
            status &= 0b1111'1101;
        }

        //if the first bit of result is 1
        if (result & 0b1000'0000) {
            //set the negative flag to 1
            status |= 0b1000'0000;
        } else {
            //set the negative flag to 0
            status &= 0b0111'1111;
        }
    }
};



