cmake_minimum_required(VERSION 3.10)
project(Lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server
    src/server.cpp
)

add_executable(client
    src/client.cpp
)

target_include_directories(server PRIVATE src)
target_include_directories(client PRIVATE src)

if (WIN32)
    target_sources(server PRIVATE src/serial_port_win.cpp)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
else()
    target_sources(server PRIVATE src/serial_port_posix.cpp)
endif()
