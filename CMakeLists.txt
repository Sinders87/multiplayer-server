cmake_minimum_required(VERSION 3.10)       # Minimum required CMake version
project(MultiplayerServer)                 # Project name

set(CMAKE_CXX_STANDARD 17)                 # Use modern C++17 features
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})  # Place executables in the build folder

include_directories(include)               # Add 'include' folder for header file lookup

# ============================================================
# SERVER EXECUTABLE
# ============================================================
add_executable(server                      # Define the 'server' build target
    src/main_server.cpp                    # Entry point for server application
    src/server.cpp                         # Server class implementation
)

target_link_libraries(server ws2_32)       # Link with Winsock2 (required for networking on Windows)

# ============================================================
# CLIENT EXECUTABLE
# ============================================================
add_executable(client                      # Define the 'client' build target
    src/main_client.cpp                    # Entry point for client application
    src/client.cpp                         # Client class implementation
)

target_link_libraries(client ws2_32)       # Link Winsock2 for the client as well
