cmake_minimum_required(VERSION 3.11)
project(AOC2019)

set(CMAKE_CXX_FLAGS "-Wall -std=c++17 -fno-omit-frame-pointer -ggdb3 -O3")

# find_package(PkgConfig)
# pkg_check_modules(PC_RapidJSON QUIET RapidJSON)

# set(BOOST_ROOT "/home/ov699/opt/")
find_package(Boost 1.67.0 COMPONENTS unit_test_framework REQUIRED)

include_directories(src ${Boost_INCLUDE_DIR})

add_executable(level1 src/level1.cc)
target_compile_features(level1 PRIVATE cxx_std_17)
