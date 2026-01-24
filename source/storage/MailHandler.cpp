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

Email MailHandler::read_from_string(const QString &mail_data) {
    Email email;
    QStringList lines = mail_data.split("\n");

    int row = 0;
    auto readCleanLine = [&](const QString &prefix) {
        return lines[row++].remove(prefix, Qt::CaseInsensitive).trimmed();
    };

    email.to = readCleanLine("To: ");
    email.from = readCleanLine("From: ");
    email.subject = readCleanLine("Subject: ");

    int pos = 0;
    for(int i = 0; i < 3; ++i)
        pos = mail_data.indexOf('\n', pos) + 1;

    email.content = mail_data.mid(pos).trimmed();

    return email;
}

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
    std::filesystem::create_directories(user_folder);

    return true;
}

void MailHandler::store_mail(const std::string &client_name, const Email &email) {
    if (!register_client(client_name)) {
        std::cerr << "[ERROR] Could not register or find folder for: " << client_name << std::endl;
        return;
    }

    std::string raw_hash = hash_mail(email);
    QString hex_hash = QString(QByteArray::fromStdString(raw_hash).toHex());

    std::string path = "../data/users/" + client_name + "/" + hex_hash.toStdString() + ".txt";
    std::ofstream file{path};

    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file for writing: " << path << std::endl;
        return;
    }

    file << "To: " << email.to.toStdString() << "\n";
    file << "From: " << email.from.toStdString() << "\n";
    file << "Subject: " << email.subject.toStdString() << "\n";

    file << email.content.toStdString() << "\n";

    file.close();
    std::cout << "[INFO] Mail stored successfully in " << client_name << "'s folder." << std::endl;
}

QVector<Email> MailHandler::get_client_mails(const std::string &client_name) {
    const std::filesystem::path directory = "../data/users/" + client_name + "/";

    std::cout << client_name;

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

        std::cout << "[INFO] Deleting mail: " << file_path << std::endl;
        if (std::ifstream file{file_path}; !file.good()) continue;

        std::filesystem::remove(file_path);
    }
}

std::string MailHandler::hash_mail(const Email &mail) {
    std::string result = hash(mail.content.toStdString());
    return result;
}
