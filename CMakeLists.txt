cmake_minimum_required(VERSION 3.12) # version can be different



project(my_NES_emulator) #name of your project

#add_subdirectory(googletest) # add googletest subdirectory

#CMake complains if I don't include this. 
cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

include_directories(googletest/include) # this is so we can #include <gtest/gtest.h>


add_executable(mytests tests.cpp NES_CPU.cpp main.cpp NES_opcodes.h) # add this executable

target_link_libraries(mytests PRIVATE gtest) # link google test to this executable