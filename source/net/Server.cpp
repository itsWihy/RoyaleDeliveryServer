//
// Created by Wihy on 12/22/25.
//

#include "../../headers/Server.h"

#include <iostream>

#include "../../headers/net/commands.h"
#include "../../headers/storage/PasswordHandler.h"

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

void Server::handle_client_data() const {
    auto *socket = qobject_cast<QTcpSocket *>(sender());

    QDataStream stream(socket->readAll());
    stream.setVersion(QDataStream::Qt_5_15);

    stream.startTransaction();

    quint32 length, cmd_type;
    stream >> length >> cmd_type;

    if (!stream.commitTransaction()) return;

    switch (cmd_type) {
        case SIGN_UP:
            QString name, password;
            stream >> name >> password;
            qDebug() << name << " and " << password;

            PasswordHandler::get_instance().insert_new_client(name.toStdString(), password.toStdString());
            send_cmd_to_client(socket, STATUS, {"SIGNUP", "TRUE"}); //todo: base response on actual bool result.
            break;
    }
    // INBOX folder, OUTBOX FOLDER>
}

bool Server::send_cmd_to_client(QTcpSocket *client, const Command cmd_type, const QStringList &parameters) {
    if (client->state() != QTcpSocket::ConnectedState) return false;

    const QByteArray data{pack_data(cmd_type, parameters)};

    return client->write(data) >= 0;
}

void Server::handle_error(const QAbstractSocket::SocketError socketError) const {
    const auto *socket = qobject_cast<QTcpSocket *>(sender());
    const char* errorMessage = nullptr;

    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            errorMessage = "Remote host closed the connection";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMessage = "The host was not found. Please check the host name and port settings";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            errorMessage = "The connection was refused by the peer. Make sure the service is running";
            break;
        case QAbstractSocket::SocketAccessError:
            errorMessage = "Socket access error (permission issue)";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorMessage = "Socket operation timed out";
            break;
        case QAbstractSocket::NetworkError:
            errorMessage = "Network error";
            break;
        default:
            errorMessage = socket ? socket->errorString().toLocal8Bit().data() : "Unknown socket error";
            break;
    }

    std::cerr << "[Socket Error] Code: " << socketError << ", Message: " << errorMessage << std::endl;
}
