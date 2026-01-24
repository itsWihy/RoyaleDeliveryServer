//
// Created by Wihy on 12/29/25.
//

#ifndef ROYALEDELIVERYSERVER_SMTPSERVER_H
#define ROYALEDELIVERYSERVER_SMTPSERVER_H
#include <qobject.h>
#include <QSslSocket>
#include <QTcpServer>

#include "SslServer.h"

enum class State {
    UNSET = 0,
    HELO = 1,
    AUTH_USER = 2,
    AUTH_PASS = 3,
    MAIL_FROM = 4,
    RCPT_TO = 5,
    DATA = 6,
    QUIT = 7
};

struct SmtpSession {
    State state = State::UNSET;
    QString total_data;
    QString sender;
    QString recipient;
    QString client_name;
    bool is_authenticated = false;
    bool is_connected = true;
};

class SMTPServer : public QObject {
    Q_OBJECT

public:
    static SMTPServer &get_instance() {
        static SMTPServer instance;
        return instance;
    }

    SMTPServer(const SMTPServer &) = delete;

    void operator=(const SMTPServer &) = delete;

public slots:
    void new_connection();
    void client_disconnected();
    void handle_client_data();
    void handle_error(QAbstractSocket::SocketError socketError) const;
    void handle_ssl_errors(const QList<QSslError> &errors) const;

private:
    SslServer server;
    QHash<QSslSocket *, SmtpSession> sessions;

    SMTPServer();

    static void handle_mail_body(QSslSocket *client, SmtpSession &session);
};


#endif //ROYALEDELIVERYSERVER_SMTPSERVER_H
