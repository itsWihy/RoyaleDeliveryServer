//
// Created by Wihy on 12/29/25.
//

#ifndef ROYALEDELIVERYSERVER_MAILHANDLER_H
#define ROYALEDELIVERYSERVER_MAILHANDLER_H
#include <filesystem>
#include <qglobal.h>
#include <string>
#include "../net/commands.h"

class MailHandler {
public:
    static MailHandler& get_instance() {
        static MailHandler instance{};
        return instance;
    }

    MailHandler(const MailHandler&) = delete;
    void operator=(const MailHandler&) = delete;

private:
    static Email read_from_file(const std::filesystem::path& path);

public:
    static bool register_client(const std::string& name);
    static void store_mail(const std::string& client_name, const std::string& mail_data);

    static QVector<Email> get_client_mails(const std::string& client_name);

    static std::string hash_mail(const std::string& mail_data);
};
#endif //ROYALEDELIVERYSERVER_MAILHANDLER_H
