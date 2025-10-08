#pragma once
// Prevents this header file from being included multiple times in one build.
// Without it, multiple includes could cause duplicate class definition errors.

#include <winsock2.h>
// Core Windows sockets library.  Defines SOCKET, bind(), listen(), accept(), etc.

#include <ws2tcpip.h>
// Provides modern helper functions for IPv4 and IPv6, such as inet_pton().

#include <string>
// Enables use of std::string for text output and error messages.

#include <iostream>
// Provides standard console I/O (std::cout, std::cerr).

#include <thread>
// Allows the server to handle multiple clients simultaneously by running
// each client connection in its own thread.

#include <vector>
// Used to keep track of all connected client sockets in a dynamic list.

#pragma comment(lib, "ws2_32.lib")
// Instructs the compiler to link the Winsock2 library, which provides all
// networking functions used by this server class.

// ============================================================
//                     CLASS DECLARATION
// ============================================================

class Server {
public:
    // Constructor:
    // Takes a port number as input and stores it for later use.
    // Example:  Server server(54000);
    Server(int port);

    // Destructor:
    // Automatically called when the Server object goes out of scope.
    // Closes all client sockets, shuts down the server socket,
    // and cleans up Winsock resources.
    ~Server();

    // start():
    // Initializes Winsock, creates the server socket, binds it to
    // the specified port, and begins listening for incoming connections.
    // Returns true if all steps succeed, false otherwise.
    bool start();

    // acceptClients():
    // Waits for incoming client connections in a loop.
    // For each new client, spawns a new thread to handle communication.
    void acceptClients();

private:
    // The port number on which the server listens for incoming connections.
    int port;

    // The main socket that listens for and accepts new client connections.
    SOCKET serverSocket;

    // A list (vector) of active client sockets currently connected to the server.
    std::vector<SOCKET> clientSockets;

    // initWinsock():
    // Helper function that initializes the Winsock library.
    // Must be called before any socket operations can occur.
    bool initWinsock();

    // handleClient():
    // Handles communication with a single connected client.
    // Runs in its own thread to allow multiple clients to be served concurrently.
    void handleClient(SOCKET clientSocket);
};
