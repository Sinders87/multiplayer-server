// Fourth file to make:  Game/Server implementation 

#include "server.hpp"
#include <algorithm>
#include <cctype>

// ============================================================
//                  SMALL STRING HELPERS
// ============================================================

namespace {
    std::string trimCopy(const std::string& s) {
        if (s.empty()) return s;

        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;

        return s.substr(start, end - start);
    }

    std::string toUpperCopy(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return out;
    }
}

// ============================================================
//                        SERVER CORE
// ============================================================

Server::Server(int port)
    : port(port), serverSocket(INVALID_SOCKET) {
    initWorld();
    bossMonster = { "Ancient Dragon", 40, 2 }; // safer demo stats
}

Server::~Server() {
    for (auto client : clientSockets)
        closesocket(client);
    closesocket(serverSocket);
    WSACleanup();
}

bool Server::initWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

bool Server::start() {
    if (!initWinsock()) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        return false;
    }

    std::cout << "Server listening on port " << port << "...\n";
    return true;
}

void Server::acceptClients() {
    while (true) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue;

        clientSockets.push_back(clientSocket);
        std::thread(&Server::handleClient, this, clientSocket).detach();

        std::cout << "[Server] New client connected (" << clientSockets.size() << " total)\n";
    }
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[512];

    // Get username
    int nameLen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (nameLen <= 0) {
        closesocket(clientSocket);
        return;
    }
    buffer[nameLen] = '\0';
    std::string username = trimCopy(buffer);
    clientNames[clientSocket] = username;

    PlayerState ps;
    ps.x = 1;
    ps.y = 1;
    ps.hp = 10;
    ps.alive = true;
    playerStates[clientSocket] = ps;

    std::cout << "[Server] " << username << " joined.\n";

    // Notify others
    std::string joinMsg = "[Server]: " + username + " has joined the world.";
    for (auto sock : clientSockets)
        if (sock != clientSocket) sendToClient(sock, joinMsg);

    // Intro
    sendToClient(clientSocket,
        "Welcome, " + username + "!\n"
        "Commands:\n"
        "  LOOK\n"
        "  MOVE <NORTH|SOUTH|EAST|WEST>\n"
        "  PICKUP <item>\n"
        "  ATTACK [monster]\n"
        "  INVENTORY\n"
        "  SAY <message>\n"
        "  BOSS (start boss demo)\n");

    handleLook(clientSocket);

    // Main loop
    while (true) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cout << "[Server] " << username << " disconnected.\n";
            closesocket(clientSocket);

            clientSockets.erase(
                std::remove(clientSockets.begin(), clientSockets.end(), clientSocket),
                clientSockets.end());
            clientNames.erase(clientSocket);
            playerStates.erase(clientSocket);
            break;
        }

        buffer[bytes] = '\0';
        std::string msg = trimCopy(buffer);
        if (msg.empty()) continue;

        if (isCommand(msg))
            handleCommand(clientSocket, username, msg);
        else
            handleChatMessage(clientSocket, username, msg);
    }
}

// ============================================================
//                     WORLD INITIALIZATION
// ============================================================

void Server::initWorld() {
    // (1,1) – center
    Room center;
    center.name = "Forest Clearing";
    center.description = "A small clearing surrounded by trees.";
    center.items = { "rusty sword" };
    center.monsters.push_back({ "rat", 5, 1 });
    center.north = center.south = center.east = center.west = true;
    world[{1, 1}] = center;

    Room north;
    north.name = "Mossy Cave Entrance";
    north.description = "A cool, damp cave entrance.";
    north.south = true;
    world[{1, 0}] = north;

    Room south;
    south.name = "Riverbank";
    south.description = "A gentle river flows nearby.";
    south.north = true;
    south.items = { "potion" };
    world[{1, 2}] = south;

    Room west;
    west.name = "Old Camp";
    west.description = "Remains of a long-abandoned camp.";
    west.east = true;
    world[{0, 1}] = west;

    Room east;
    east.name = "Rocky Path";
    east.description = "A narrow, rocky trail.";
    east.west = true;
    east.monsters.push_back({ "goblin", 8, 2 });
    world[{2, 1}] = east;

    Room boss;
    boss.name = "Ancient Sanctum";
    boss.description = "A vast stone chamber humming with dark energy.";
    boss.north = boss.south = boss.east = boss.west = false;
    world[{BOSS_X, BOSS_Y}] = boss;
}

// ============================================================
//                 LOOKUP / UTILITY HELPERS
// ============================================================

PlayerState* Server::getPlayerState(SOCKET clientSocket) {
    auto it = playerStates.find(clientSocket);
    if (it == playerStates.end()) return nullptr;
    return &it->second;
}

Room* Server::getPlayerRoom(SOCKET clientSocket) {
    PlayerState* ps = getPlayerState(clientSocket);
    if (!ps) return nullptr;

    auto it = world.find({ ps->x, ps->y });
    if (it == world.end()) return nullptr;
    return &it->second;
}

void Server::sendToClient(SOCKET clientSocket, const std::string& message) {
    if (clientSocket == INVALID_SOCKET) return;
    std::string data = message;
    if (data.empty() || data.back() != '\n')
        data.push_back('\n');
    send(clientSocket, data.c_str(), (int)data.size(), 0);
}

void Server::broadcastToRoom(int x, int y, const std::string& message, SOCKET excludeSocket) {
    for (const auto& [sock, st] : playerStates) {
        if (sock == excludeSocket) continue;
        if (st.x == x && st.y == y)
            sendToClient(sock, message);
    }
}

// ============================================================
//                  COMMAND PARSING / ROUTING
// ============================================================

bool Server::isCommand(const std::string& message) const {
    std::string upper = toUpperCopy(trimCopy(message));
    if (upper.empty()) return false;

    if (upper == "LOOK" || upper == "INVENTORY" || upper == "HELP" ||
        upper == "BOSS" || upper == "ATTACK")
        return true;

    if (upper.rfind("MOVE ", 0) == 0) return true;
    if (upper.rfind("ATTACK ", 0) == 0) return true;
    if (upper.rfind("PICKUP ", 0) == 0) return true;
    if (upper.rfind("SAY ", 0) == 0) return true;

    return false;
}

void Server::handleCommand(SOCKET clientSocket,
                           const std::string& username,
                           const std::string& command) {
    std::string trimmed = trimCopy(command);
    std::string upper = toUpperCopy(trimmed);

    size_t spacePos = upper.find(' ');
    std::string cmd = (spacePos == std::string::npos) ? upper : upper.substr(0, spacePos);
    std::string arg = (spacePos == std::string::npos) ? "" : trimmed.substr(spacePos + 1);

    if (cmd == "LOOK") {
        handleLook(clientSocket);
    } else if (cmd == "INVENTORY") {
        handleInventory(clientSocket);
    } else if (cmd == "HELP") {
        handleHelp(clientSocket);
    } else if (cmd == "BOSS") {
        startBossFight();
    } else if (cmd == "MOVE") {
        handleMove(clientSocket, arg);
    } else if (cmd == "PICKUP") {
        handlePickup(clientSocket, arg);
    } else if (cmd == "ATTACK") {
        handleAttack(clientSocket, arg);
    } else if (cmd == "SAY") {
        handleChatMessage(clientSocket, username, arg);
    } else {
        sendToClient(clientSocket, "[Server] Unknown command.");
    }
}

// ============================================================
//                    INDIVIDUAL COMMANDS
// ============================================================

void Server::handleHelp(SOCKET clientSocket) {
    std::string msg;
    msg += "=== Help ===\n";
    msg += "LOOK                - Describe the room\n";
    msg += "MOVE <dir>          - Move NORTH/SOUTH/EAST/WEST\n";
    msg += "PICKUP <item>       - Take an item\n";
    msg += "ATTACK [monster]    - Attack a monster in the room\n";
    msg += "INVENTORY           - Show HP and items\n";
    msg += "SAY <message>       - Talk to players in the room\n";
    msg += "BOSS                - Start the boss demo\n";
    sendToClient(clientSocket, msg);
}

void Server::handleLook(SOCKET clientSocket) {
    Room* room = getPlayerRoom(clientSocket);
    PlayerState* ps = getPlayerState(clientSocket);
    if (!room || !ps) {
        sendToClient(clientSocket, "[Server] You are nowhere. (Bug)");
        return;
    }

    std::string msg;
    msg += "=== " + room->name + " ===\n";
    msg += room->description + "\n\n";

    // Items
    if (!room->items.empty()) {
        msg += "Items:\n";
        for (const auto& item : room->items)
            msg += " - " + item + "\n";
    } else {
        msg += "No items here.\n";
    }

    // Monsters
    if (!room->monsters.empty()) {
        msg += "\nMonsters:\n";
        for (const auto& m : room->monsters)
            msg += " - " + m.name + " (" + std::to_string(m.hp) + " HP)\n";
    }

    // Players
    msg += "\nPlayers:\n";
    for (const auto& [sock, st] : playerStates) {
        if (st.x == ps->x && st.y == ps->y) {
            auto it = clientNames.find(sock);
            std::string name = (it != clientNames.end()) ? it->second : "Unknown";
            msg += " - " + name + (sock == clientSocket ? " (you)\n" : "\n");
        }
    }

    // Exits
    msg += "\nExits: ";
    bool any = false;
    if (room->north) { msg += "North"; any = true; }
    if (room->south) { msg += (any ? ", " : "") + std::string("South"); any = true; }
    if (room->east)  { msg += (any ? ", " : "") + std::string("East");  any = true; }
    if (room->west)  { msg += (any ? ", " : "") + std::string("West");  any = true; }
    if (!any) msg += "None";
    msg += "\n";

    sendToClient(clientSocket, msg);
}

void Server::handleMove(SOCKET clientSocket, const std::string& direction) {
    PlayerState* ps = getPlayerState(clientSocket);
    if (!ps) return;

    Room* room = getPlayerRoom(clientSocket);
    if (!room) {
        sendToClient(clientSocket, "[Server] You seem to be lost.");
        return;
    }

    std::string dir = toUpperCopy(trimCopy(direction));
    int newX = ps->x;
    int newY = ps->y;
    std::string dirWord;

    if (dir == "NORTH") {
        if (!room->north) { sendToClient(clientSocket, "[Server] You can't go that way."); return; }
        newY -= 1; dirWord = "north";
    } else if (dir == "SOUTH") {
        if (!room->south) { sendToClient(clientSocket, "[Server] You can't go that way."); return; }
        newY += 1; dirWord = "south";
    } else if (dir == "EAST") {
        if (!room->east) { sendToClient(clientSocket, "[Server] You can't go that way."); return; }
        newX += 1; dirWord = "east";
    } else if (dir == "WEST") {
        if (!room->west) { sendToClient(clientSocket, "[Server] You can't go that way."); return; }
        newX -= 1; dirWord = "west";
    } else {
        sendToClient(clientSocket, "[Server] Unknown direction.");
        return;
    }

    if (world.find({ newX, newY }) == world.end()) {
        sendToClient(clientSocket, "[Server] There's nothing that way.");
        return;
    }

    std::string name = clientNames[clientSocket];
    broadcastToRoom(ps->x, ps->y, name + " leaves " + dirWord + ".", clientSocket);

    ps->x = newX;
    ps->y = newY;

    broadcastToRoom(ps->x, ps->y, name + " arrives.", clientSocket);

    sendToClient(clientSocket, "You move " + dirWord + ".");
    handleLook(clientSocket);
}

void Server::handleInventory(SOCKET clientSocket) {
    PlayerState* ps = getPlayerState(clientSocket);
    if (!ps) return;

    std::string msg = "=== Inventory ===\nHP: " + std::to_string(ps->hp) + "\n";
    if (ps->inventory.empty()) {
        msg += "You are not carrying anything.\n";
    } else {
        msg += "You are carrying:\n";
        for (const auto& item : ps->inventory)
            msg += " - " + item + "\n";
    }
    sendToClient(clientSocket, msg);
}

void Server::handlePickup(SOCKET clientSocket, const std::string& itemName) {
    PlayerState* ps = getPlayerState(clientSocket);
    Room* room = getPlayerRoom(clientSocket);
    if (!ps || !room) {
        sendToClient(clientSocket, "[Server] You are nowhere.");
        return;
    }

    std::string item = trimCopy(itemName);
    if (item.empty()) {
        sendToClient(clientSocket, "[Server] Usage: PICKUP <item>");
        return;
    }

    auto it = std::find(room->items.begin(), room->items.end(), item);
    if (it == room->items.end()) {
        sendToClient(clientSocket, "[Server] That item is not here.");
        return;
    }

    ps->inventory.push_back(*it);
    room->items.erase(it);

    std::string name = clientNames[clientSocket];
    sendToClient(clientSocket, "You pick up the " + item + ".");
    broadcastToRoom(ps->x, ps->y, name + " picks up the " + item + ".", clientSocket);
}

void Server::handleAttack(SOCKET clientSocket, const std::string& targetName) {
    PlayerState* ps = getPlayerState(clientSocket);
    if (!ps) return;

    // Auto-route to boss in boss room
    if (ps->x == BOSS_X && ps->y == BOSS_Y && bossActive) {
        handleBossAttack(clientSocket, "ANCIENT DRAGON");
        return;
    }

    Room* room = getPlayerRoom(clientSocket);
    if (!room) {
        sendToClient(clientSocket, "[Server] You are nowhere.");
        return;
    }

    if (!ps->alive) {
        sendToClient(clientSocket, "[Server] You are currently dead.");
        return;
    }

    std::string target = trimCopy(targetName);
    if (target.empty()) {
        if (room->monsters.empty()) {
            sendToClient(clientSocket, "[Server] There is nothing to attack here.");
            return;
        }
        target = room->monsters[0].name;
    }

    std::string targetUpper = toUpperCopy(target);
    int index = -1;
    for (size_t i = 0; i < room->monsters.size(); ++i) {
        if (toUpperCopy(room->monsters[i].name) == targetUpper) {
            index = (int)i;
            break;
        }
    }

    if (index == -1) {
        sendToClient(clientSocket, "[Server] No such monster here.");
        return;
    }

    Monster& m = room->monsters[index];

    int playerDamage = 12;
    m.hp -= playerDamage;

    std::string name = clientNames[clientSocket];
    broadcastToRoom(ps->x, ps->y, name + " attacks the " + m.name + "!", clientSocket);

    std::string msg = "You attack the " + m.name + " for " +
        std::to_string(playerDamage) + " damage.\n";

    if (m.hp <= 0) {
        msg += "The " + m.name + " dies!\n";
        room->monsters.erase(room->monsters.begin() + index);
        sendToClient(clientSocket, msg);
        return;
    }

    int monsterDamage = m.damage;
    ps->hp -= monsterDamage;
    msg += "The " + m.name + " hits you for " +
        std::to_string(monsterDamage) + ".\n";

    if (ps->hp <= 0) {
        ps->alive = false;
        msg += "You have died!\n";
        broadcastToRoom(ps->x, ps->y, name + " falls, defeated.", clientSocket);

        ps->x = 1;
        ps->y = 1;
        ps->hp = 40;
        ps->alive = true;

        msg += "You wake up back at the Forest Clearing.\n";
        sendToClient(clientSocket, msg);
        handleLook(clientSocket);
        return;
    }

    msg += "Your HP is now " + std::to_string(ps->hp) + ".\n";
    sendToClient(clientSocket, msg);
}

// ============================================================
//                      BOSS FIGHT SYSTEM
// ============================================================

void Server::startBossFight() {
    if (bossActive) return;

    bossActive = true;
    bossMonster = { "Ancient Dragon", 40, 2 };

    for (auto& [sock, state] : playerStates)
        sendToClient(sock, "\nA dark presence stirs... You are pulled into the Ancient Sanctum!\n");

    // Save positions and teleport everyone
    for (auto& [sock, state] : playerStates) {
        savedPositions[sock] = { state.x, state.y };
        state.x = BOSS_X;
        state.y = BOSS_Y;
    }

    // Show the boss as a monster in the room (for LOOK)
    Room& bossRoom = world[{ BOSS_X, BOSS_Y }];
    bossRoom.monsters.clear();
    bossRoom.monsters.push_back(bossMonster);

    for (auto& [sock, _] : playerStates)
        handleLook(sock);

    broadcastToRoom(BOSS_X, BOSS_Y,
        "The Ancient Dragon descends from the shadows!");
}

void Server::handleBossAttack(SOCKET clientSocket, const std::string& targetName) {
    if (toUpperCopy(targetName) != "ANCIENT DRAGON") {
        sendToClient(clientSocket, "[Server] You can only attack the Ancient Dragon!");
        return;
    }

    int playerDamage = 12;
    bossMonster.hp -= playerDamage;

    broadcastToRoom(BOSS_X, BOSS_Y,
        clientNames[clientSocket] + " strikes the Ancient Dragon!");

    if (bossMonster.hp <= 0) {
        broadcastToRoom(BOSS_X, BOSS_Y, "The Ancient Dragon collapses!");
        bossActive = false;
        returnPlayersFromBossRoom();
        return;
    }

    // Boss counterattack (attacker only)
    playerStates[clientSocket].hp -= bossMonster.damage;

    broadcastToRoom(BOSS_X, BOSS_Y,
        "The Ancient Dragon retaliates against " + clientNames[clientSocket] + "!");

    if (playerStates[clientSocket].hp <= 0) {
        playerStates[clientSocket].alive = false;
        broadcastToRoom(BOSS_X, BOSS_Y,
            clientNames[clientSocket] + " has been slain by the Ancient Dragon!");
    }

    broadcastToRoom(BOSS_X, BOSS_Y,
        "Boss HP: " + std::to_string(bossMonster.hp));
}

void Server::returnPlayersFromBossRoom() {
    broadcastToRoom(BOSS_X, BOSS_Y,
        "A warm light envelops you as you return to your world...");

    for (auto& [sock, st] : playerStates) {
        if (st.x == BOSS_X && st.y == BOSS_Y) {
            auto oldPos = savedPositions[sock];
            st.x = oldPos.first;
            st.y = oldPos.second;

            sendToClient(sock, "You have returned to your previous location.\n");
            handleLook(sock);
        }
    }

    savedPositions.clear();
}

// ============================================================
//                         CHAT SYSTEM
// ============================================================

void Server::handleChatMessage(SOCKET clientSocket,
                               const std::string& username,
                               const std::string& message) {
    PlayerState* ps = getPlayerState(clientSocket);

    std::string tagged = "[" + username + "]: " + message;
    std::cout << tagged << "\n";

    if (!ps) {
        // Fallback to global chat if no state
        for (auto sock : clientSockets)
            if (sock != clientSocket) sendToClient(sock, tagged);
        return;
    }

    broadcastToRoom(ps->x, ps->y, tagged, clientSocket);
    sendToClient(clientSocket, "You say: " + message);
}
