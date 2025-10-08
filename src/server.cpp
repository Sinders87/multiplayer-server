// =============================================================
//  File: server.cpp
//  Description: Implements the Server class responsible for
//  initializing Winsock, handling TCP connections, and
//  communicating with connected clients.
// =============================================================

#include "server.hpp"  // Include class definition and dependencies

// -------------------------------------------------------------
// Constructor: initializes the server port and marks the main
// server socket as INVALID_SOCKET (not yet created).
// -------------------------------------------------------------
Server::Server(int port) : port(port), serverSocket(INVALID_SOCKET) {}

// -------------------------------------------------------------
// Destructor: closes all open sockets and cleans up Winsock.
// -------------------------------------------------------------
Server::~Server() {
    for (auto client : clientSockets)
        closesocket(client);        // Close each client socket
    closesocket(serverSocket);      // Close the main server socket
    WSACleanup();                   // Release Winsock resources
}

// -------------------------------------------------------------
// Initializes the Winsock library (required for network I/O).
// Returns true if successful, false if an error occurs.
// -------------------------------------------------------------
bool Server::initWinsock() {
    WSADATA wsaData;                            // Holds Winsock version info
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

// -------------------------------------------------------------
// Starts the server: initializes Winsock, creates a socket,
// binds it to the desired port, and begins listening.
// -------------------------------------------------------------
bool Server::start() {
    if (!initWinsock()) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    // Create a TCP socket (IPv4 + stream type)
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    // Setup the server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;           // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;   // Listen on all interfaces
    serverAddr.sin_port = htons(port);         // Convert port to network byte order

    // Bind the socket to the address and port
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return false;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        return false;
    }

    std::cout << "Server listening on port " << port << "...\n";
    return true;
}

// -------------------------------------------------------------
// Continuously accepts incoming clients and spawns a thread
// to handle each one concurrently.
// -------------------------------------------------------------
void Server::acceptClients() {
    while (true) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);

        // Wait for a client to connect
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue;

        // Store the client socket and handle it in a new thread
        clientSockets.push_back(clientSocket);
        std::thread(&Server::handleClient, this, clientSocket).detach();

        std::cout << "New client connected!\n";
    }
}

// -------------------------------------------------------------
// Handles communication with a single connected client.
// Receives messages, prints them to the console, and closes
// the connection when the client disconnects.
// -------------------------------------------------------------
void Server::handleClient(SOCKET clientSocket) {
    char buffer[512];  // Message buffer

    while (true) {
        // Wait for data from the client
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        // If connection closed or error, break loop
        if (bytesReceived <= 0) {
            std::cout << "Client disconnected.\n";
            closesocket(clientSocket);
            break;
        }

        // Null-terminate and display the received message
        buffer[bytesReceived] = '\0';
        std::cout << "Client says: " << buffer << std::endl;
    }
}
