cmake_minimum_required(VERSION 3.11)
project(AOC2019)

#  set(CMAKE_CXX_FLAGS "-Wall -Wshadow -std=c++20 -Wpedantic -fno-omit-frame-pointer -ggdb3 -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -std=c++20 -Wpedantic -fno-omit-frame-pointer -ggdb3 -O2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find_package(PkgConfig)
# pkg_check_modules(PC_RapidJSON QUIET RapidJSON)

#  find_package(TBB REQUIRED tbb)

# set(BOOST_ROOT "/home/ov699/opt/")
find_package(Boost 1.67.0 COMPONENTS unit_test_framework REQUIRED)

include_directories(src ${Boost_INCLUDE_DIR} ${TBB_DIR})

add_executable(level1 src/level1.cc)

add_executable(level3 src/level3.cc)

add_executable(level4 src/level4.cc)

add_executable(level5 src/level5.cc)

add_executable(level6_1 src/level6_1.cc)

add_executable(level6_2 src/level6_2.cc)

add_executable(level7 src/level7.cc)

add_executable(level8 src/level8.cc)

add_executable(level9 src/level9.cc)

add_executable(level10 src/level10.cc)

add_executable(level11 src/level11.cc)

add_executable(level12 src/level12.cc)

add_executable(level13 src/level13.cc)

add_executable(level14 src/level14.cc)

add_executable(level15 src/level15.cc)

add_executable(level16 src/level16.cc)
#  target_link_libraries(level16 PRIVATE TBB::tbb)

add_executable(level17 src/level17.cc)

add_executable(level18 src/level18.cc)

add_executable(level19 src/level19.cc)

add_executable(level20 src/level20.cc)

add_executable(level21 src/level21.cc)

add_executable(level23 src/level23.cc)

add_executable(level24 src/level24.cc)
