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

Email MailHandler::read_from_file(const std::filesystem::path &path) {
    Email email;
    std::ifstream file{path};
    std::string line;

    auto readCleanLine = [&](const QString &prefix) {
        if (std::string l; getline(file, l)) {
            QString qline = QString::fromStdString(l);
            return qline.remove(prefix, Qt::CaseInsensitive).trimmed();
        }

        return QString();
    };

    email.to = readCleanLine("To: ");
    email.from = readCleanLine("From: ");
    email.subject = readCleanLine("Subject: ");

    std::string data;
    while (getline(file, line))
        data += line + "\n";

    email.content = QString::fromStdString(data).trimmed();

    return email;
}

bool MailHandler::register_client(const std::string &name) {
    if (!ClientHandler::get_instance().has_client(name)) return false;

    const std::filesystem::path user_folder{"../data/users/" + name};
    return std::filesystem::create_directories(user_folder);
}

void MailHandler::store_mail(const std::string &client_name, const Email &email) {
    const auto mail_file_name = hash_mail(email);
    const auto hex_hash = QByteArray{mail_file_name.c_str()};

    std::ofstream file{"../data/users/" + client_name + "/" + hex_hash.toHex().toStdString() + ".txt"};

    file << email.content.toStdString();

    file << std::endl;
    file.close();
}

QVector<Email> MailHandler::get_client_mails(const std::string &client_name) {
    const std::filesystem::path directory = "../data/users/" + client_name + "/";

    QVector<Email> mails;

    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(directory)) {
        Email mail{read_from_file(dirEntry.path())};
        mails.append(mail);
    }

    return mails;
}

void MailHandler::delete_mail(const QString &hash) {
    const std::filesystem::path directory = "../data/users/";

    for (const auto &dirEntry: std::filesystem::directory_iterator(directory)) {
        auto hex_hash = QByteArray{hash.toStdString().c_str()};
        std::filesystem::path file_path{dirEntry.path().string() + "/" + hex_hash.toHex().toStdString() + ".txt"};

        if (std::ifstream file{file_path}; !file.good()) continue;

        std::filesystem::remove(file_path);
    }
}

std::string MailHandler::hash_mail(const Email &mail) {
    std::string result = hash(mail.content.toStdString());
    return result;
}
