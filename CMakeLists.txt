cmake_minimum_required(VERSION 3.16)
project(counter)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(counter src/main.cpp)

target_link_libraries(counter ${CONAN_LIBS})