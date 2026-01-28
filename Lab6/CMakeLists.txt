cmake_minimum_required(VERSION 3.10)
project(Lab6)

add_definitions(-D_WIN32_WINNT=0x0A00)
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
endif()

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

target_link_libraries(server ws2_32)
target_link_libraries(client ws2_32)

set(GUI_SOURCES
    src/main.cpp
    src/TemperatureGraph.cpp
    src/HTTPClient.cpp
)

add_executable(gui_app ${GUI_SOURCES})

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Network)

target_include_directories(gui_app PRIVATE src external)

target_link_libraries(gui_app
    SFML::Graphics
    SFML::Window
    SFML::System
	SFML::Network
    ws2_32
)