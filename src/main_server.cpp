#include "server.hpp"
// Includes the Server class declaration so this file can
// create a Server object and use its public functions.
// The header defines the functions start() and acceptClients().

// ============================================================
//                      MAIN FUNCTION
// ============================================================

int main() {
    // Create a Server object that listens on port 54000.
    // This port must match the one used by the clients.
    Server server(54000);

    // Attempt to start the server.
    // start() handles all setup work: initializing Winsock,
    // creating the listening socket, binding it to the port,
    // and putting it into listening mode.
    // If any of those steps fail, return 1 to signal an error.
    if (!server.start()) return 1;

    // Begin accepting incoming client connections.
    // This function will usually run indefinitely,
    // waiting for new clients and handling them as they connect.
    server.acceptClients();

    // If the program ever reaches this point (for example, if the server is stopped),
    // return 0 to indicate successful completion.
    return 0;
}
