cmake_minimum_required(VERSION 3.10)
project(MultiplayerServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(server
    src/main_server.cpp
    src/server.cpp
    src/packet.cpp
)

add_executable(client
    src/main_client.cpp
    src/client.cpp
    src/packet.cpp
)

# Link Winsock (for sockets on Windows)
target_link_libraries(server ws2_32)
target_link_libraries(client ws2_32)
