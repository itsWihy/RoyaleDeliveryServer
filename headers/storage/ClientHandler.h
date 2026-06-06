/**
 * @file ClientHandler.h
 * @brief Header file for the ClientHandler class, which manages user credentials and active client mappings.
 */

#ifndef ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#define ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#include <qstring.h>
#include <QTcpSocket>
#include <unordered_map>

/**
 * @class ClientHandler
 * @brief Singleton class that handles user authentication and maps network clients to their usernames.
 */
class ClientHandler {
public:
    /**
     * @brief Gets the singleton instance of the ClientHandler.
     * @return Reference to the ClientHandler instance.
     */
    static ClientHandler& get_instance() {
        static ClientHandler instance{};
        return instance;
    }

    ClientHandler(const ClientHandler&) = delete;
    void operator=(const ClientHandler&) = delete;

private:
    std::unordered_map<std::string, std::string> client_to_password; ///< Maps usernames to their hashed passwords
    std::unordered_map<std::string, std::string> client_ip_to_name{}; ///< Maps client IP/descriptors to usernames

    /**
     * @brief Private constructor for the Singleton pattern.
     */
    ClientHandler();

public:
    /**
     * @brief Loads user credential data from a persistent file.
     */
    void load_from_file();

    /**
     * @brief Writes user credential data to a persistent file.
     */
    void write_to_file();

    /**
     * @brief Checks if a client with the given name already exists.
     * @param name The username to check.
     * @return True if the user exists, false otherwise.
     */
    bool has_client(const std::string& name);

    /**
     * @brief Registers a new client with a username and password.
     * @param name The desired username.
     * @param password The plaintext password (will be hashed).
     * @return True if the registration was successful, false if the user already exists.
     */
    bool insert_new_client(const std::string& name, const std::string& password);

    /**
     * @brief Validates a plaintext password for a given username.
     * @param name The username.
     * @param password The plaintext password to validate.
     * @return True if the password matches, false otherwise.
     */
    bool is_password_valid(const std::string& name, const std::string& password);

    /**
     * @brief Validates a pre-hashed password for a given username.
     * @param name The username.
     * @param hashed_password The hashed password to validate.
     * @return True if the hashed password matches, false otherwise.
     */
    bool is_hashed_password_valid(const std::string& name, const std::string& hashed_password);

    /**
     * @brief Associates an active socket connection with a username.
     * @param client Pointer to the QTcpSocket.
     * @param name The username to associate.
     */
    void insert_ip_to_client(const QTcpSocket* client, const std::string& name);

    /**
     * @brief Retrieves the username associated with a given socket connection.
     * @param client Pointer to the QTcpSocket.
     * @return The associated username, or an empty string if not found.
     */
    std::string get_name_from_client(const QTcpSocket *client) const;

};

#endif //ROYALEDELIVERYSERVER_PASSWORDHANDLER_H