cmake_minimum_required(VERSION 3.3)
project(format)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Test.cpp)
add_executable(Test ${SOURCE_FILES})
add_library(format format.cpp)

target_link_libraries(Test format)

install(TARGETS format DESTINATION lib)
install(FILES format.h DESTINATION include)