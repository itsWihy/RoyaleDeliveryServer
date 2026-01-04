//
// Created by Wihy on 12/29/25.
//

#include "../../headers/storage/MailHandler.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <qvector.h>

#include "../../headers/Utilities.h"
#include "../../headers/storage/ClientHandler.h"

Email MailHandler::read_from_file(const std::filesystem::path& path) {
    Email email;
    std::ifstream file{path};
    std::string line;

    auto readCleanLine = [&](const QString &prefix) {
        if (std::string l; getline(file,l)) {
            QString qline = QString::fromStdString(l);
            return qline.remove(prefix, Qt::CaseInsensitive).trimmed();
        }

        return QString();
    };

    email.to = readCleanLine("To: ");
    email.from =  readCleanLine("From: ");
    email.subject =  readCleanLine("Subject: ");

    std::string data;
    while (getline(file, line))
        data += line + "\n";

    email.content = QString::fromStdString(data).trimmed();

    return email;
}

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

QVector<Email> MailHandler::get_client_mails(const std::string &client_name) {
    const std::filesystem::path directory = "../data/users/" + client_name + "/";

    QVector<Email> mails;

    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(directory)) {
        std::cout << dirEntry << std::endl;

        Email mail {read_from_file(dirEntry.path())};
        mails.append(mail);
    }

    return mails;
}

std::string MailHandler::hash_mail(const std::string &mail_data) {
    std::string result = hash(mail_data);
    const auto time =  std::chrono::system_clock::now().time_since_epoch().count();

    result.append(std::to_string(time));

    return result;
}
