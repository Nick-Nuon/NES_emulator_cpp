#ifndef NES_CPU_HPP
#define NES_CPU_HPP

#include <cstdint>
#include <vector>

class CPU {
public:
    CPU();
    void interpret(const std::vector<uint8_t>& program);
    uint8_t register_a;
    uint8_t status;
    uint16_t program_counter;
    uint8_t register_x;

private:
    void lda(uint8_t value);
    void tax();
    void inx();
    void update_zero_and_negative_flags(uint8_t result);
};

#endif // NES_CPU_HPP
