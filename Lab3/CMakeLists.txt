cmake_minimum_required(VERSION 3.10)
project(Lab3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab3 main.cpp)

if (UNIX)
    target_link_libraries(lab3 pthread rt)
endif()
