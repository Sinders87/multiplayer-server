//Fourth file to make
#include "client.hpp"   // Includes the Client class definition
#include <thread>       // Enables background message listening using threads

// ============================================================
//                 CLIENT IMPLEMENTATION
// ============================================================

// Constructor — stores server IP and port, sets socket as invalid
Client::Client(const std::string& serverIP, int port)
    : serverIP(serverIP), port(port), clientSocket(INVALID_SOCKET) {}

// Destructor — closes the socket and cleans up Winsock
Client::~Client() {
    closesocket(clientSocket);  // Gracefully close the socket connection
    WSACleanup();               // Shut down the Winsock library
}

// Initializes the Winsock library (required before any socket operations)
bool Client::initWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;  // Returns true if startup succeeded
}

// Attempts to connect to the server using stored IP and port
bool Client::connectToServer() {
    if (!initWinsock()) {  // Ensure Winsock is initialized first
        std::cerr << "[Client] Failed to initialize Winsock.\n";
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);  // Create a TCP socket
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "[Client] Socket creation failed.\n";
        return false;
    }

    sockaddr_in serverAddr{};                        // Define server address structure
    serverAddr.sin_family = AF_INET;                 // IPv4
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr); // Convert IP string → binary
    serverAddr.sin_port = htons(port);               // Convert port to network byte order

    // Attempt to connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Client] Connection failed.\n";
        return false;
    }

    std::cout << "[Client] Connected to server!\n";

    // Prompt user for a username and send it to the server
    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);
    send(clientSocket, username.c_str(), username.size(), 0);

    return true;
}

// Sends a text message to the server
void Client::sendMessage(const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}

// Continuously listens for messages from the server in a background thread
void Client::listenForMessages() {
    char buffer[512];  // Message buffer
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {  // 0 = disconnected, <0 = error
            std::cout << "[Client] Disconnected from server.\n";
            break;
        }

        buffer[bytesReceived] = '\0';  // Null-terminate received message
        std::cout << "\n[Server Broadcast] " << buffer << "\n> ";
        std::cout.flush();             // Keep prompt clean in console
    }
}
