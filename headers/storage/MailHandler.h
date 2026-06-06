/**
 * @file MailHandler.h
 * @brief Header file for the MailHandler class, which manages email storage and retrieval.
 */

#ifndef ROYALEDELIVERYSERVER_MAILHANDLER_H
#define ROYALEDELIVERYSERVER_MAILHANDLER_H
#include <filesystem>
#include <qglobal.h>
#include <string>
#include "../net/commands.h"

/**
 * @class MailHandler
 * @brief Singleton class that handles reading, storing, deleting, and retrieving emails from the file system.
 */
class MailHandler {
public:
    /**
     * @brief Gets the singleton instance of the MailHandler.
     * @return Reference to the MailHandler instance.
     */
    static MailHandler& get_instance() {
        static MailHandler instance{};
        return instance;
    }

    MailHandler(const MailHandler&) = delete;
    void operator=(const MailHandler&) = delete;

private:
    /**
     * @brief Reads an email from a specific file path.
     * @param path The filesystem path to the email file.
     * @return An Email object populated with the file's data.
     */
    static Email read_from_file(const std::filesystem::path& path);

public:
    /**
     * @brief Parses an email from a raw string (e.g., received via SMTP).
     * @param mail_data The raw email content as a QString.
     * @return An Email object populated with the parsed data.
     */
    static Email read_from_string(const QString &mail_data);

    /**
     * @brief Creates a storage directory for a new client.
     * @param name The username of the client.
     * @return True if successful, false otherwise.
     */
    static bool register_client(const std::string& name);

    /**
     * @brief Stores an email in the file system for a specific client.
     * @param client_name The username of the client whose inbox the email belongs to.
     * @param email The Email object to store.
     */
    static void store_mail(const std::string& client_name, const Email& email);

    /**
     * @brief Deletes an email file based on its hash.
     * @param hash The unique hash of the email to delete.
     */
    static void delete_mail(const QString &hash);

    /**
     * @brief Retrieves all emails stored for a specific client.
     * @param client_name The username of the client.
     * @return A QVector of Email objects.
     */
    static QVector<Email> get_client_mails(const std::string& client_name);

    /**
     * @brief Generates a unique hash for an email based on its contents.
     * @param mail The Email object to hash.
     * @return A string representing the hash of the email.
     */
    static std::string hash_mail(const Email& mail);
};
#endif //ROYALEDELIVERYSERVER_MAILHANDLER_H