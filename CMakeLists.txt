cmake_minimum_required(VERSION 3.16)
project(counter)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

find_package(Boost)
find_package(fmt)
add_executable(counter src/main.cpp)

target_link_libraries(counter PRIVATE Boost::boost fmt::fmt-header-only)