//
// Created by Wihy on 12/29/25.
//

#include "../../headers/storage/MailHandler.h"

#include <filesystem>
#include <fstream>

#include "../../headers/Utilities.h"
#include "../../headers/storage/ClientHandler.h"

bool MailHandler::register_client(const std::string& name) {
    if (!ClientHandler::get_instance().has_client(name)) return false;

    const std::filesystem::path user_folder {"../data/users/" + name};
    return std::filesystem::create_directories(user_folder);
}

void MailHandler::store_mail(const std::string& client_name, const std::string& mail_data) {
    const auto mail_file_name = hash_mail(mail_data);

    std::ofstream file{"../data/users/" + client_name + "/" + mail_file_name + ".txt"};

    file << mail_data;

    file << std::endl;
    file.close();
}

std::string MailHandler::hash_mail(const std::string &mail_data) {
    std::string result = hash(mail_data);
    const auto time =  std::chrono::system_clock::now().time_since_epoch().count();

    result.append(std::to_string(time));

    return result;
}
