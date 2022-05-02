cmake_minimum_required(VERSION 3.22)
project(trpp2a_new)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

add_executable(trpp2a_new main.cpp AhoCorasick.hpp)
