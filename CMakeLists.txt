cmake_minimum_required(VERSION 3.19)
project(SicAssembler)

set(CMAKE_CXX_STANDARD 11)

add_executable(SicAssembler sic.cpp)
add_executable(Test test01.cpp)