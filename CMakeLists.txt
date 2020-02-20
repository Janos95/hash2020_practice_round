cmake_minimum_required(VERSION 3.15)
project(pizza_slices)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OsiSym REQUIRED IMPORTED_TARGET osi-sym)
set(CMAKE_CXX_STANDARD 20)

add_executable(pizza_slices main.cpp parse_input.h fs.hpp)
target_link_libraries(pizza_slices PRIVATE
        PkgConfig::OsiSym
        Threads::Threads
        )