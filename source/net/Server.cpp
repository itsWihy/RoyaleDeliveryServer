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
    connect(client, &QTcpSocket::readyRead, this, &Server::handle_client_data);
    connect(client, &QAbstractSocket::errorOccurred, this, &Server::handle_error);

    clients.push_back(client);

    qDebug() << "Socket connected from " + ipAddress + ":" + QString::number(port);
}

void Server::client_disconnected() const {
    const QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    const QString client_ip_addr = client->peerAddress().toString();
    const int client_port = client->peerPort();

    qDebug() << "Socket disconnected from " + client_ip_addr + ":" + QString::number(client_port);
}

void Server::handle_client_data() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());

    //HANDLE:
    // sign up.
    // log in.
    // actual mail.
    //TODO: Correctly handle DATA. 
    QDataStream stream(socket->readAll());
    stream.setVersion(QDataStream::Qt_5_15);

    stream.startTransaction();

    QByteArray cmd, name, pass;
    stream >> cmd >> name >> pass;
    qDebug() << cmd << " And " << name << " andndd" << pass;

    if (!stream.commitTransaction()) return;

    socket->write("Message received by Server sdfjosdijf. I wodner how long I can make this until aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaauaaavaaawaaaxaaayaaazaabbaabcaabdaabeaabfaabgaabhaabiaabjaabkaablaabmaabnaaboaabpaabqaabraabsaabtaabuaabvaabwaabxaabyaab");
}

void Server::handle_error(QAbstractSocket::SocketError socketError) const {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    std::string errorMsg = socket ? socket->errorString().toStdString() : "Unknown error";

    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            std::cout << "Error: The host was not found. Please check the host name and port settings." << std::endl;
            break;
        case QAbstractSocket::ConnectionRefusedError:
            std::cout << "Error: The connection was refused by the peer. Make sure the service is running." <<
                    std::endl;
            break;
        default:
            std::cout << "The following error occurred: " << errorMsg << std::endl;
            break;
    }
}
