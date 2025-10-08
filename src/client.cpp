#include "client.hpp"
// Includes the client class definition from client.hpp so that the
// compiler knows which functions are being implemented below.

// ============================================================
//                CONSTRUCTOR AND DESTRUCTOR
// ============================================================

Client::Client(const std::string& serverIP, int port)
    : serverIP(serverIP), port(port), clientSocket(INVALID_SOCKET) {}
// Constructor initialization list:
//   • Copies the given server IP and port into the class members.
//   • Sets clientSocket to INVALID_SOCKET to indicate that no
//     connection has been established yet.

Client::~Client() {
    closesocket(clientSocket);   // Closes the socket connection if open.
    WSACleanup();                // Frees all resources allocated by Winsock.
}
// The destructor ensures that network resources are properly released
// when the client object goes out of scope or the program exits.

// ============================================================
//                WINSOCK INITIALIZATION
// ============================================================

bool Client::initWinsock() {
    WSADATA wsaData;                         // Structure to hold information about the Winsock implementation.
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}
// Initializes the Winsock library with version 2.2.
// Returns true if initialization succeeds, or false if it fails.

// ============================================================
//                CONNECT TO SERVER
// ============================================================

bool Client::connectToServer() {
    // Step 1: Initialize Winsock.
    if (!initWinsock()) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    // Step 2: Create a TCP socket.
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    // Step 3: Configure the server’s address structure.
    sockaddr_in serverAddr{};                   // Zero-initialize the structure.
    serverAddr.sin_family = AF_INET;            // Set address family to IPv4.
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    // Converts the string IP (e.g., "127.0.0.1") into a numeric binary form.
    serverAddr.sin_port = htons(port);          // Converts port to network byte order.

    // Step 4: Attempt to connect to the server.
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        return false;
    }

    // Step 5: If connection succeeds, inform the user.
    std::cout << "Connected to server!\n";
    return true;
}

// ============================================================
//                SEND MESSAGE TO SERVER
// ============================================================

void Client::sendMessage(const std::string& message) {
    // Sends the provided message string to the connected server.
    // message.c_str() gives a C-style string pointer.
    // message.size() is the number of bytes to send.
    // The final argument (flags) is set to 0.
    send(clientSocket, message.c_str(), message.size(), 0);
}
