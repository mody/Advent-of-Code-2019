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

add_executable(level2 src/level2.cc)
target_compile_features(level2 PRIVATE cxx_std_17)

add_executable(level3 src/level3.cc)
target_compile_features(level3 PRIVATE cxx_std_17)

add_executable(level4 src/level4.cc)
target_compile_features(level4 PRIVATE cxx_std_17)

add_executable(level5 src/level5.cc)
target_compile_features(level5 PRIVATE cxx_std_17)

add_executable(level6_1 src/level6_1.cc)
target_compile_features(level6_1 PRIVATE cxx_std_17)

add_executable(level6_2 src/level6_2.cc)
target_compile_features(level6_2 PRIVATE cxx_std_17)

add_executable(level7 src/level7.cc)
target_compile_features(level7 PRIVATE cxx_std_17)

add_executable(level8 src/level8.cc)
target_compile_features(level8 PRIVATE cxx_std_17)

add_executable(level9 src/level9.cc)
target_compile_features(level9 PRIVATE cxx_std_17)

add_executable(level10 src/level10.cc)
target_compile_features(level10 PRIVATE cxx_std_17)
