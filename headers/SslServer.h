//
// Created by Wihy on 1/24/26.
//

#ifndef ROYALEDELIVERYSERVER_SSLSERVER_H
#define ROYALEDELIVERYSERVER_SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

class SslServer : public QTcpServer {
    Q_OBJECT
//openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes -subj "/CN=192.168.1.156"
protected:
    void incomingConnection(const qintptr socketDescriptor) override {
        if (auto *socket = new QSslSocket(this); socket->setSocketDescriptor(socketDescriptor)) {
            addPendingConnection(socket);
        } else {
            delete socket;
        }
    }
};

#endif //ROYALEDELIVERYSERVER_SSLSERVER_H
