//
// Created by Wihy on 12/24/25.
//

#include "../../headers/storage/ClientHandler.h"

#include <fstream>
#include <iostream>
#include <qhostaddress.h>

#include "QCryptographicHash"
#include "../../headers/Utilities.h"
#include "../../headers/storage/MailHandler.h"

/**
 * @brief Default constructor for ClientHandler.
 * Initializes the handler by loading client data from the persistent storage file.
 */
ClientHandler::ClientHandler() {
    load_from_file();
}

/**
 * @brief Loads client credentials from the local passwords file.
 * 
 * This function reads "../data/passwords.txt", parses each line (format: client_name:password),
 * and populates the internal client_to_password map.
 */
void ClientHandler::load_from_file() {
    std::ifstream file{"../data/passwords.txt"};
    std::string line;

    // Read file line by line
    while (getline(file, line)) {
        if (line.empty()) continue;

        // Each line is expected to be in "username:hashed_password" format
        size_t position = line.find(':');
        if (position == std::string::npos) continue;

        const std::string client_name{line.substr(0, position)};
        const std::string password{line.substr(position + 1)};

        if (client_name.empty() || password.empty()) continue;

        std::cout << "[INFO] Loaded " << client_name << " with pass " << password << "\n";
        // Insert into the map, avoiding overwriting if somehow duplicate
        client_to_password.try_emplace(client_name, password);
    }

    file.close();
}

/**
 * @brief Persists the current client_to_password map to the local passwords file.
 * 
 * Overwrites the "../data/passwords.txt" file with the current in-memory data.
 */
void ClientHandler::write_to_file() {
    std::ofstream file{"../data/passwords.txt"};
    file.clear(); // Clear existing content

    for (auto &[name, hashed_pass]: client_to_password) {
        if (name.empty() || hashed_pass.empty()) continue;
        file << name << ":"  << hashed_pass << "\n";
    }

    file.close();
}

/**
 * @brief Checks if a client with the given name exists.
 * 
 * @param name The name of the client to check.
 * @return true If the client exists, false otherwise.
 */
bool ClientHandler::has_client(const std::string &name) {
    return client_to_password.find(name) != client_to_password.end();
}

/**
 * @brief Registers a new client with a name and password.
 * 
 * @param name The unique name for the new client.
 * @param password The plain-text password (will be hashed before storage).
 * @return true If registration was successful, false if the client already exists.
 */
bool ClientHandler::insert_new_client(const std::string &name, const std::string &password) {
    if (has_client(name)) return false;

    // Hash the password before storing for security
    const bool result = client_to_password.try_emplace(name, hash(password)).second;
    
    // Save the new client to disk immediately
    write_to_file();

    // Create necessary mail infrastructure for the new client
    MailHandler::register_client(name);

    return result;
}

/**
 * @brief Validates a client's credentials using a plain-text password.
 * 
 * @param name The client's name.
 * @param password The plain-text password to verify.
 * @return true If credentials match, false otherwise.
 */
bool ClientHandler::is_password_valid(const std::string &name, const std::string &password) {
    if (!has_client(name)) return false;

    // Compare stored hash with the hash of the provided password
    return client_to_password.find(name)->second == hash(password);
}

/**
 * @brief Validates a client's credentials using a pre-hashed password.
 * 
 * @param name The client's name.
 * @param hashed_password The hashed password to verify.
 * @return true If credentials match, false otherwise.
 */
bool ClientHandler::is_hashed_password_valid(const std::string &name, const std::string &hashed_password) {
    if (!has_client(name)) return false;

    return client_to_password.find(name)->second == hashed_password;
}

/**
 * @brief Maps a connected socket's IP address and port to a client name.
 * 
 * Used to track which authenticated user is communicating over which socket.
 * 
 * @param client Pointer to the QTcpSocket of the connected client.
 * @param name The authenticated client name.
 */
void ClientHandler::insert_ip_to_client(const QTcpSocket* client, const std::string &name) {
    const std::string identifier = client->peerAddress().toString().toStdString() + ":" + std::to_string(client->peerPort());
    client_ip_to_name.insert_or_assign(identifier, name);
}

/**
 * @brief Retrieves the client name associated with a specific socket's IP and port.
 * 
 * @param client Pointer to the QTcpSocket.
 * @return std::string The name of the client associated with the socket, or empty if not found.
 */
std::string ClientHandler::get_name_from_client(const QTcpSocket *client) const {
    const std::string identifier = client->peerAddress().toString().toStdString() + ":" + std::to_string(client->peerPort());
    auto it = client_ip_to_name.find(identifier);
    if (it != client_ip_to_name.end()) {
        return it->second;
    }
    return "";
}
