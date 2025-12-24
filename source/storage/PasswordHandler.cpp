//
// Created by Wihy on 12/24/25.
//

#include "../../headers/storage/PasswordHandler.h"

#include <fstream>
#include <iostream>

PasswordHandler::PasswordHandler() {
    load_from_file();
}

//side-effects: change std_hash_map.
void PasswordHandler::load_from_file() {
    std::ifstream file{"../data/passwords.txt"};
    std::string line;

    while (getline(file, line)) {
        const std::string client_name{line.substr(0, line.find_first_of(':'))};
        const std::string password{line.substr(line.find_first_of(':') + 1, line.size() - 1)};

        if (client_name.empty() || password.empty()) continue;

        std::cout << "[INFO] Loaded " << client_name << " with pass " << password << " and also " << static_cast<int>(line.find_first_of('c')) << "\n";
        client_to_password.try_emplace(client_name, password);
    }

    file.close();
}

void PasswordHandler::write_to_file() {
    std::ofstream file{"../data/passwords.txt"};
    file.clear(); //todo: So efficient. delete and revert each time!

    for (auto &pair: client_to_password) {
        if (pair.first.empty() || pair.second.empty()) continue;
        //todo: LOL PLAIN TEXT PASSWORD
        file << pair.first << ":" << "" << pair.second << "\n";
    }

    file << std::endl;
    file.close();
}

bool PasswordHandler::insert_new_client(const std::string &name, const std::string &password) {
    if (client_to_password.find(name) != client_to_password.end()) return false;

    const bool result = client_to_password.try_emplace(name, password).second;
    write_to_file();

    return result;
}

bool PasswordHandler::check_pass_validity(const std::string &name, const std::string &password) {
    if (client_to_password.find(name) == client_to_password.end()) return false;

    return client_to_password.find(name)->second == password;
}
