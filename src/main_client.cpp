#include "client.hpp"
// Includes the Client class definition so we can create a Client object
// and call its member functions (connectToServer, sendMessage, etc.).

#include <string>
// Includes the string library so we can use std::string to store user input.

// ============================================================
//                      MAIN FUNCTION
// ============================================================

int main() {
    // Create a Client object that connects to localhost (127.0.0.1)
    // on port 54000. This means it will try to connect to a server
    // running on the same machine, listening on that port.
    Client client("127.0.0.1", 54000);

    // Attempt to connect to the server.
    // If the connection fails, exit the program with status 1.
    if (!client.connectToServer()) return 1;

    // Variable to hold the message typed by the user.
    std::string message;

    // Infinite loop — keeps running until the user types "quit".
    while (true) {
        std::cout << "> ";             // Print a prompt to indicate ready for input.
        std::getline(std::cin, message); // Read a full line of text from the user.

        if (message == "quit") break;   // If the user types "quit", exit the loop.

        client.sendMessage(message);    // Otherwise, send the typed message to the server.
    }

    // Once the loop ends, main returns 0 to indicate success.
    return 0;
}
