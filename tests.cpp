#include <gtest/gtest.h>
#include "NES_CPU.cpp"

class CPUTest : public testing::Test {
protected:
    void SetUp() override {
        cpu_ = std::make_unique<CPU>();
    }

    void TearDown() override {
        cpu_.reset();
    }

    std::unique_ptr<CPU> cpu_;
};

TEST_F(CPUTest, LDA_Immediate_LoadData) {
    cpu_->run({0xa9, 0x05, 0x00});
    ASSERT_EQ(cpu_->register_a, 0x05);
    ASSERT_FALSE(cpu_->status & 0b0000'0010);
    ASSERT_FALSE(cpu_->status & 0b1000'0000);
}

TEST_F(CPUTest, LDA_ZeroFlag) {
    cpu_->run({0xa9, 0x00, 0x00});
    ASSERT_TRUE(cpu_->status & 0b0000'0010);
}

TEST_F(CPUTest, TAX_MoveAtoX) {
    cpu_->register_a = 10;
    cpu_->run({0xaa, 0x00});
    ASSERT_EQ(cpu_->register_x, 10);
}

TEST_F(CPUTest, FiveOpsWorkingTogether) {
    cpu_->run({0xa9, 0xc0, 0xaa, 0xe8, 0x00});
    ASSERT_EQ(cpu_->register_x, 0xc1);
}

TEST_F(CPUTest, INX_Overflow) {
    cpu_->register_x = 0xff;
    cpu_->run({0xe8, 0xe8, 0x00});
    ASSERT_EQ(cpu_->register_x, 1);
}
