//First file to make
//Declares the Server class (acts as a blueprint for how the server behaves)

#pragma once // Prevents this header file from being included multiple times during compilation

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <string>       
#include <iostream>     
#include <thread>       
#include <vector>       
#include <unordered_map>
#include <algorithm>    
#include <map>          

#pragma comment(lib, "ws2_32.lib")

// ============================================================
//                   GAME-RELATED STRUCTS
// ============================================================

// Represents a monster in a room
struct Monster {
    std::string name;
    int hp;
    int damage;
};

// Represents a room in the MUD world
struct Room {
    std::string name;
    std::string description;
    std::vector<std::string> items;
    std::vector<Monster> monsters;

    bool north = false;
    bool south = false;
    bool east  = false;
    bool west  = false;
};

// Represents a player's in-game state
struct PlayerState {
    int x = 1;
    int y = 1;
    int hp = 40;
    bool alive = true;
    std::vector<std::string> inventory;
};

// Boss room coordinates
const int BOSS_X = 9;
const int BOSS_Y = 9;

// ============================================================
//                       SERVER CLASS
// ============================================================

class Server {
public:
    Server(int port);
    ~Server();

    bool start();
    void acceptClients();

private:
    // Networking / core state
    int port;
    SOCKET serverSocket;
    std::vector<SOCKET> clientSockets;
    std::unordered_map<SOCKET, std::string> clientNames;
    std::unordered_map<SOCKET, PlayerState> playerStates;

    // 2D world map of rooms
    std::map<std::pair<int,int>, Room> world;

    // Boss system state (NOW CORRECTLY MEMBERS OF SERVER)
    std::unordered_map<SOCKET, std::pair<int,int>> savedPositions;
    Monster bossMonster;
    bool bossActive = false;

    // --- Core network setup ---
    bool initWinsock();
    void handleClient(SOCKET clientSocket);
    void initWorld();

    // --- Command parsing / routing ---
    bool isCommand(const std::string& message) const;

    void handleCommand(SOCKET clientSocket,
                       const std::string& username,
                       const std::string& command);

    void handleLook(SOCKET clientSocket);
    void handleMove(SOCKET clientSocket, const std::string& direction);
    void handleInventory(SOCKET clientSocket);
    void handlePickup(SOCKET clientSocket, const std::string& itemName);
    void handleAttack(SOCKET clientSocket, const std::string& targetName);
    void handleChatMessage(SOCKET clientSocket,
                           const std::string& username,
                           const std::string& message);
    void handleHelp(SOCKET clientSocket);

    //Boss fight system 
    void startBossFight();
    void handleBossAttack(SOCKET clientSocket, const std::string& targetName);
    void returnPlayersFromBossRoom();

    //Helper functions
    void sendToClient(SOCKET clientSocket, const std::string& message);
    void broadcastToRoom(int x, int y, const std::string& message,
                         SOCKET excludeSocket = INVALID_SOCKET);

    Room* getPlayerRoom(SOCKET clientSocket);
    PlayerState* getPlayerState(SOCKET clientSocket);
};

