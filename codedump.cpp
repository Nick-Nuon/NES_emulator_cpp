#include <cstdint>
#include <vector>
#include <cassert>
#include <iostream>
#include "gtest/gtest.h"
#include <type_traits>

#include <bitset>
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


class CPU {
public:
    uint8_t status = 0b0000'0000; 

    void remove_flag(CpuFlags flag){
        status &= ~static_cast<uint8_t>(flag);
        std::cout << status;
    };
    void add_flag(CpuFlags flag){
        status |= static_cast<uint8_t>(flag);
        std::cout << status;
    };
};


int main() {


  return 0;
}
