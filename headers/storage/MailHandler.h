//
// Created by Wihy on 12/29/25.
//

#ifndef ROYALEDELIVERYSERVER_MAILHANDLER_H
#define ROYALEDELIVERYSERVER_MAILHANDLER_H
#include <qglobal.h>
#include <string>




class MailHandler {
public:
    static MailHandler& get_instance() {
        static MailHandler instance{};
        return instance;
    }

    MailHandler(const MailHandler&) = delete;
    void operator=(const MailHandler&) = delete;


public:
    bool register_client(const std::string& name); //create the folder if doesn't exist.
    void store_mail(const std::string& client_name, const std::string& mail_data);

    static std::string hash_mail(const std::string& mail_data);
};
#endif //ROYALEDELIVERYSERVER_MAILHANDLER_H
