cmake_minimum_required(VERSION 3.10)
project(Lab5)

set(CMAKE_CXX_STANDARD 17)

set(SERVER_SOURCES
    src/server.cpp
    src/db.cpp
    src/sqlite3.c
)

set(CLIENT_SOURCES
    src/client.cpp
)

if(WIN32)
    list(APPEND SERVER_SOURCES src/serial_port_win.cpp)
    list(APPEND CLIENT_SOURCES src/serial_port_win.cpp)
else()
    list(APPEND SERVER_SOURCES src/serial_port_posix.cpp)
    list(APPEND CLIENT_SOURCES src/serial_port_posix.cpp)
endif()

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()
