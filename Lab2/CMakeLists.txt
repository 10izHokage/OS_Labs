cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(process_lib
    lib/process_lib_posix.cpp
    lib/process_lib_win.cpp)

target_include_directories(process_lib PUBLIC lib)

add_executable(test_process main.cpp)

target_link_libraries(test_process process_lib)
