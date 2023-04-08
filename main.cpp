#include <iostream>
#include "gtest/gtest.h"
#pragma once


//int main(int, char**) {
int main(int argc, char **argv) {

    std::cout << "Hello, world!\n";

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
