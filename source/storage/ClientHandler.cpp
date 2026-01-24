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

ClientHandler::ClientHandler() {
    load_from_file();
}

//side-effects: change std_hash_map.
void ClientHandler::load_from_file() {
    std::ifstream file{"../data/passwords.txt"};
    std::string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t position = line.find(':');
        if (position == std::string::npos) continue;

        const std::string client_name{line.substr(0, position)};
        const std::string password{line.substr(position + 1)};

        if (client_name.empty() || password.empty()) continue;

        std::cout << "[INFO] Loaded " << client_name << " with pass " << password << "\n";
        client_to_password.try_emplace(client_name, password);
    }

    file.close();
}

void ClientHandler::write_to_file() {
    std::ofstream file{"../data/passwords.txt"};
    file.clear();

    for (auto &[name, hashed_pass]: client_to_password) {
        if (name.empty() || hashed_pass.empty()) continue;
        file << name << ":"  << hashed_pass << "\n";
    }

    file.close();
}

bool ClientHandler::has_client(const std::string &name) {
    return client_to_password.find(name) != client_to_password.end();
}

bool ClientHandler::insert_new_client(const std::string &name, const std::string &password) {
    if (has_client(name)) return false;

    const bool result = client_to_password.try_emplace(name, hash(password)).second;
    write_to_file();

    MailHandler::register_client(name);

    return result;
}

bool ClientHandler::is_password_valid(const std::string &name, const std::string &password) {
    if (!has_client(name)) return false;

    return client_to_password.find(name)->second == hash(password);
}

bool ClientHandler::is_hashed_password_valid(const std::string &name, const std::string &hashed_password) {
    if (!has_client(name)) return false;

    return client_to_password.find(name)->second == hashed_password;
}

void ClientHandler::insert_ip_to_client(const QTcpSocket* client, const std::string &name) {
    client_ip_to_name.insert_or_assign(client->peerAddress().toString().toStdString(), name);
}

std::string ClientHandler::get_name_from_client(const QTcpSocket *client) const {
        const auto ip_from_client = client->peerAddress().toString();
        return client_ip_to_name.at(ip_from_client.toStdString());
}
