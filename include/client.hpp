#pragma once 
// Prevents this header file from being included multiple times in a single build.
// Without this, if two or more files include "client.hpp", the compiler would see
// multiple definitions of the Client class and throw an error.

#include <winsock2.h>   
// Core Windows sockets library.  Defines SOCKET, connect(), send(), recv(), etc.

#include <ws2tcpip.h>   
// Adds modern helper functions for IPv4 and IPv6, such as inet_pton().

#include <string>       
// Enables use of std::string for storing text messages and IP addresses.

#include <iostream>     
// Provides standard input/output functions like std::cout and std::cin.

#pragma comment(lib, "ws2_32.lib")  
// Instructs the compiler to link against the Winsock library (ws2_32.lib).
// This library contains all the networking functions used by Winsock.

// ============================================================
//                     CLASS DECLARATION
// ============================================================

class Client {
public:
    // Constructor:
    // Takes the server’s IP address and port number as arguments.
    // Example usage:
    //      Client client("127.0.0.1", 54000);
    Client(const std::string& serverIP, int port);

    // Destructor:
    // Automatically called when the Client object goes out of scope.
    // Will close the socket connection and clean up Winsock resources.
    ~Client();

    // connectToServer():
    // Initializes Winsock, creates a socket, and attempts to connect
    // to the specified server using the stored IP and port.
    // Returns true on success, false on failure.
    bool connectToServer();

    // sendMessage():
    // Sends a text message string over the connected socket to the server.
    void sendMessage(const std::string& message);

private:
    // The IP address of the server (for example, "127.0.0.1" for localhost)
    std::string serverIP;

    // The server’s port number (for example, 54000)
    int port;

    // The socket object used for the client’s network connection
    SOCKET clientSocket;

    // initWinsock():
    // Helper function that initializes the Winsock library.
    // Must be called before any socket operations can occur.
    bool initWinsock();
};
