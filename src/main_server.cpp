//Fifth file to make
#include "server.hpp"   // Includes the Server class definition for creating and controlling the server

// ============================================================
//                      MAIN FUNCTION
// ============================================================

int main() {
    Server server(54000);               // Create a Server object that listens on port 54000 (must match client port)
    if (!server.start()) return 1;      // Initialize Winsock, bind, and start listening; exit if setup fails
    server.acceptClients();             // Continuously accept and handle incoming client connections (runs indefinitely)

    return 0;                           // Normal exit if the server loop ever ends
}
