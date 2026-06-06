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

/**
 * @brief Parses an email from a raw string format.
 * 
 * Expected format:
 * To: <recipient>
 * From: <sender>
 * Subject: <subject>
 * <content>
 * 
 * @param mail_data The raw email string.
 * @return Email The parsed Email object.
 */
Email MailHandler::read_from_string(const QString &mail_data) {
    Email email;
    QStringList lines = mail_data.split("\n");

    int row = 0;
    // Lambda to extract value after a prefix and increment the current line index
    auto readCleanLine = [&](const QString &prefix) {
        return lines[row++].remove(prefix, Qt::CaseInsensitive).trimmed();
    };

    email.to = readCleanLine("To: ");
    email.from = readCleanLine("From: ");
    email.subject = readCleanLine("Subject: ");

    // Finding the start of the body after the first 3 lines (headers)
    int pos = 0;
    for(int i = 0; i < 3; ++i)
        pos = mail_data.indexOf('\n', pos) + 1;

    email.content = mail_data.mid(pos).trimmed();

    return email;
}

/**
 * @brief Reads an email from a file on disk.
 * 
 * @param path The filesystem path to the email file.
 * @return Email The parsed Email object.
 */
Email MailHandler::read_from_file(const std::filesystem::path &path) {
    Email email;
    std::ifstream file{path};
    std::string line;

    // Lambda to read a line from ifstream and clean it based on prefix
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

    // The rest of the file is the email body
    std::string data;
    while (getline(file, line))
        data += line + "\n";

    email.content = QString::fromStdString(data).trimmed();

    return email;
}

/**
 * @brief Ensures a client's storage directory exists.
 * 
 * @param name The client's username.
 * @return true If the directory exists or was successfully created.
 */
bool MailHandler::register_client(const std::string &name) {
    const std::filesystem::path user_folder{"../data/users/" + name};

    std::error_code ec;
    // Creates directory and all necessary parents
    std::filesystem::create_directories(user_folder, ec);

    return std::filesystem::exists(user_folder);
}

/**
 * @brief Stores an email into a client's specific folder.
 * 
 * The email is saved as a .txt file named after the hash of its content.
 * 
 * @param client_name The recipient's name.
 * @param email The Email object to store.
 */
void MailHandler::store_mail(const std::string &client_name, const Email &email) {
    if (!register_client(client_name)) {
        std::cerr << "[ERROR] Could not register or find folder for: " << client_name << std::endl;
        return;
    }

    // Hash the mail content to generate a unique filename
    std::string raw_hash = hash_mail(email);
    QString hex_hash = QString(QByteArray::fromStdString(raw_hash).toHex());

    std::string path = "../data/users/" + client_name + "/" + hex_hash.toStdString() + ".txt";
    std::ofstream file{path};

    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file for writing: " << path << std::endl;
        return;
    }

    // Write headers followed by the body
    file << "To: " << email.to.toStdString() << "\n";
    file << "From: " << email.from.toStdString() << "\n";
    file << "Subject: " << email.subject.toStdString() << "\n";

    file << email.content.toStdString() << "\n";

    file.close();
    std::cout << "[INFO] Mail stored successfully in " << client_name << "'s folder." << std::endl;
}

/**
 * @brief Retrieves all emails stored in a client's folder.
 * 
 * @param client_name The client's name.
 * @return QVector<Email> A list of all Email objects found.
 */
QVector<Email> MailHandler::get_client_mails(const std::string &client_name) {
    const std::filesystem::path directory = "../data/users/" + client_name + "/";

    std::cout << client_name;

    QVector<Email> mails;

    // Iterate through all files in the user's directory
    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(directory)) {
        Email mail{read_from_file(dirEntry.path())};
        mails.append(mail);
    }

    return mails;
}

/**
 * @brief Deletes an email with a specific hash from all user folders.
 * 
 * @param hash The hex hash string identifying the mail.
 */
void MailHandler::delete_mail(const QString &hash) {
    const std::filesystem::path directory = "../data/users/";

    // Search through all user directories to find and delete the file matching the hash
    for (const auto &dirEntry: std::filesystem::directory_iterator(directory)) {
        auto hex_hash = QByteArray{hash.toStdString().c_str()};
        std::filesystem::path file_path{dirEntry.path().string() + "/" + hex_hash.toHex().toStdString() + ".txt"};

        std::cout << "[INFO] Deleting mail: " << file_path << std::endl;
        // Check if file exists before attempting removal
        if (std::ifstream file{file_path}; !file.good()) continue;

        std::filesystem::remove(file_path);
    }
}

/**
 * @brief Generates a hash for an email based on its content.
 * 
 * @param mail The Email object.
 * @return std::string The resulting hash.
 */
std::string MailHandler::hash_mail(const Email &mail) {
    std::string result = hash(mail.content.toStdString());
    return result;
}
