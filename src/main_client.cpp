//Sixth file to make

#include "client.hpp"   // Includes the Client class definition
#include <string>       // For handling message input
#include <thread>       // Used to run the background listener thread

int main() {
    Client client("127.0.0.1", 54000);      // Create a client that connects to localhost on port 54000
    if (!client.connectToServer()) return 1; // Attempt connection; exit if it fails

    // Start a background thread that constantly listens for messages from the server
    std::thread listener(&Client::listenForMessages, &client);
    listener.detach();  // Detach so it runs independently while main handles input

    std::string message;
    while (true) {
        std::cout << "> ";                   // Display input prompt
        std::getline(std::cin, message);     // Read user input
        if (message == "quit") break;        // Exit loop if user types "quit"
        client.sendMessage(message);         // Send message to the server
    }

    return 0;  // Clean exit once user quits
}
