//
// Created by Wihy on 12/24/25.
//

#ifndef ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#define ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#include <qstring.h>
#include <unordered_map>

class ClientHandler {
public:
    static ClientHandler& get_instance() {
        static ClientHandler instance{};
        return instance;
    }

    ClientHandler(const ClientHandler&) = delete;
    void operator=(const ClientHandler&) = delete;

private:
    std::unordered_map<std::string, std::string> client_to_password;

    ClientHandler();

public:
    void load_from_file();
    void write_to_file();

    bool has_client(const std::string& name);
    bool insert_new_client(const std::string& name, const std::string& password);
    bool is_password_valid(const std::string& name, const std::string& password);
};

#endif //ROYALEDELIVERYSERVER_PASSWORDHANDLER_H