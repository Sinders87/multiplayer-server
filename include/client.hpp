//Second file to make
#pragma once // Prevents this header file from being included multiple times during compilation

#include <winsock2.h>    // Core Windows Sockets API (defines SOCKET, connect(), send(), recv(), etc.)
#include <ws2tcpip.h>    // Adds modern IPv4/IPv6 helper functions (e.g., inet_pton for address conversion)
#include <string>        // Enables use of std::string for handling text and IP addresses
#include <iostream>      // Provides standard input/output stream functions (std::cout, std::cin)

#pragma comment(lib, "ws2_32.lib") // Links the Winsock2 library that provides actual networking implementations

// ============================================================
//                      CLASS DECLARATION
// ============================================================

class Client {
public:
    Client(const std::string& serverIP, int port); // Constructor — stores the server IP and port number
    ~Client();                                    // Destructor — closes the socket and cleans up Winsock on exit

    bool connectToServer();                       // Initializes Winsock, creates a socket, and connects to the server
    void sendMessage(const std::string& message); // Sends a text message to the connected server
    void listenForMessages();                     // Listens continuously for incoming messages from the server

private:
    std::string serverIP;                         // The server's IP address (e.g., "127.0.0.1" for localhost)
    int port;                                     // The port number to connect to (e.g., 54000)
    SOCKET clientSocket;                          // Socket object representing the client's connection

    bool initWinsock();                           // Initializes the Winsock library before using sockets
};
