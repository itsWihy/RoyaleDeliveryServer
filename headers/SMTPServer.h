//
// Created by Wihy on 12/29/25.
//

#ifndef ROYALEDELIVERYSERVER_SMTPSERVER_H
#define ROYALEDELIVERYSERVER_SMTPSERVER_H
#include <qobject.h>
#include <QTcpServer>

enum class State {
    UNSET = 0,
    HELO = 1,
    MAIL_FROM = 2,
    RCPT_TO = 3,
    DATA = 4
};

struct client_status {
    bool is_connected;
    bool has_started_transaction;
};

//expose SMTP connection access, receive emails and store them here. Validate format and stuff ig
class SMTPServer : public QObject {
    Q_OBJECT

public:
    static SMTPServer &get_instance() {
        static SMTPServer instance;
        return instance;
    }

    SMTPServer(const SMTPServer &) = delete;

    void operator=(const SMTPServer &) = delete;

    //receive SMTP packet and dissect them. Does need to ALSO send packets, but do that later. RN only recv.
public slots:
    //return value is cosmetic.
    bool new_connection();

    void client_disconnected();

    void handle_client_data();

    void handle_error(QAbstractSocket::SocketError socketError) const;

private:
    QTcpServer server;
    State state;

    SMTPServer();

    void handle_mail_body(QTcpSocket *client);

public:
    std::unordered_map<QTcpSocket *, client_status> keep_connection;
};


#endif //ROYALEDELIVERYSERVER_SMTPSERVER_H
