cmake_minimum_required(VERSION 3.5)
project(task6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp lazy_string.cpp lazy_string.h)
add_executable(task6 ${SOURCE_FILES})