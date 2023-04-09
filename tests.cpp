#include <gtest/gtest.h>
#include "NES_CPU.cpp"

#include <gtest/gtest.h>

class CPUTest : public ::testing::Test {
protected:
    CPU cpu;

    virtual void SetUp() {
        // This method is called before each test is executed
    }

    virtual void TearDown() {
        // This method is called after each test is executed
    }
};

TEST_F(CPUTest, ResetSetsRegistersToInitialState) {
    cpu.reset();

    EXPECT_EQ(0, cpu.register_a);
    EXPECT_EQ(0, cpu.register_x);
    EXPECT_EQ(0, cpu.register_y);
    EXPECT_EQ(STACK_RESET, cpu.stack_pointer);
    EXPECT_EQ(0b00100100, cpu.status);
}

TEST_F(CPUTest, LoadProgramIntoMemory) {
    std::vector<uint8_t> program = {0xA9, 0x01, 0x85, 0x02};

    cpu.load(program);

    for (size_t i = 0; i < program.size(); ++i) {
        EXPECT_EQ(program[i], cpu.memory[0x8000 + i]);
    }
}

TEST_F(CPUTest, LoadAndRunProgram) {
    std::vector<uint8_t> program = {0xA9, 0x01, 0x85, 0x02, 0x00};

    cpu.load_and_run(program);

    EXPECT_EQ(1, cpu.register_a);
    EXPECT_EQ(1, cpu.memory[0x8002]);
}

TEST_F(CPUTest, AddFlag) {
    cpu.status = 0;
    cpu.add_flag(CpuFlags::Zero);

    EXPECT_EQ(static_cast<uint8_t>(CpuFlags::Zero), cpu.status);
}

TEST_F(CPUTest, RemoveFlag) {
    cpu.status = static_cast<uint8_t>(CpuFlags::Zero);
    cpu.remove_flag(CpuFlags::Zero);

    EXPECT_EQ(0, cpu.status);
}
