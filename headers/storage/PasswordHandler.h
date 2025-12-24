//
// Created by Wihy on 12/24/25.
//

#ifndef ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#define ROYALEDELIVERYSERVER_PASSWORDHANDLER_H
#include <qstring.h>
#include <unordered_map>

class PasswordHandler {
public:
    static PasswordHandler& get_instance() {
        static PasswordHandler instance{};
        return instance;
    }

    PasswordHandler(const PasswordHandler&) = delete;
    void operator=(const PasswordHandler&) = delete;

private:
    std::unordered_map<std::string, std::string> client_to_password;

    PasswordHandler();

public:
    void load_from_file();
    void write_to_file();

    bool insert_new_client(const std::string& name, const std::string& password);
    bool check_pass_validity(const std::string& name, const std::string& password);
};

#endif //ROYALEDELIVERYSERVER_PASSWORDHANDLER_H