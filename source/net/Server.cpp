//
// Created by Wihy on 12/22/25.
//

#include "../../headers/Server.h"

#include <iostream>

#include "../../headers/net/conversions.h"

Server::Server() : server(this) {
    server.listen(QHostAddress::Any, 5004);

    connect(&server, &QTcpServer::newConnection, this, &Server::new_connection);
}

void Server::new_connection() {
    QTcpSocket *client = server.nextPendingConnection();

    const QString ipAddress = client->peerAddress().toString();
    const int port = client->peerPort();

    connect(client, &QTcpSocket::disconnected, this, &Server::client_disconnected);
    connect(client, &QTcpSocket::readyRead,this, &Server::handle_client_data);

    clients.push_back(client);

    qDebug() << "Socket connected from " + ipAddress + ":" + QString::number(port);
}

void Server::client_disconnected() const {
    const QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());

    const QString client_ip_addr = client->peerAddress().toString();
    const int client_port = client->peerPort();

    qDebug() << "Socket disconnected from " + client_ip_addr + ":" + QString::number(client_port);
}

void Server::handle_client_data() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());

    QString data = "";

    QByteArray buffer;
    qint32 size = 0;

    QDataStream client_stream(client);
    //todo: Idk why QIODevice::read (QBuffer): WriteOnly device
    // search online. stop fucking around idiot

    while (client->bytesAvailable() > 0) {
        buffer.append(client->readAll());

        while ((size == 0 && buffer.size() >= 4) || (size > 0 && buffer.size() >= size)) {
            if (size == 0 && buffer.size() >= 4) {
                size = byte_array_to_int( buffer.mid(0, 4));
                buffer.remove(0, 4);
            }

            if (size > 0 && buffer.size() >= size) {
                data = QString::fromUtf8(buffer.mid(0, size));
            }
        }
    }

    std::cout << "Received " << data.data() << "";
}
