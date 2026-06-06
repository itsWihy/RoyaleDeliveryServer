/**
 * @file SslServer.h
 * @brief Header file for the SslServer class, a custom QTcpServer that supports SSL.
 */

#ifndef ROYALEDELIVERYSERVER_SSLSERVER_H
#define ROYALEDELIVERYSERVER_SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

/**
 * @class SslServer
 * @brief A custom QTcpServer that overrides incomingConnection to use QSslSocket.
 * 
 * To generate a self-signed certificate for testing:
 * openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes -subj "/CN=192.168.1.156"
 */
class SslServer : public QTcpServer {
    Q_OBJECT

protected:
    /**
     * @brief Overrides QTcpServer::incomingConnection to handle new connections using QSslSocket.
     * @param socketDescriptor The native socket descriptor for the incoming connection.
     */
    void incomingConnection(const qintptr socketDescriptor) override {
        // Create a new QSslSocket to handle the SSL connection
        if (auto *socket = new QSslSocket(this); socket->setSocketDescriptor(socketDescriptor)) {
            // Add the socket to the list of pending connections
            addPendingConnection(socket);
        } else {
            // If setting the descriptor fails, delete the socket to avoid memory leaks
            delete socket;
        }
    }
};

#endif //ROYALEDELIVERYSERVER_SSLSERVER_H