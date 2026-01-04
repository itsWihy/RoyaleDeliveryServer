//
// Created by Wihy on 12/22/25.
//

#include "../../headers/Server.h"

#include <iostream>

#include "../../headers/net/commands.h"
#include "../../headers/storage/ClientHandler.h"
#include "../../headers/storage/MailHandler.h"

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
        case SIGN_UP: {
            QString name, password;
            stream >> name >> password;

            const bool result = ClientHandler::get_instance().insert_new_client(name.toStdString(), password.toStdString());
            send_cmd_to_client<QString>(socket, STATUS, {"SIGNUP", result == 0 ? "FALSE" : "TRUE"});

            if (result == 1)
                ClientHandler::get_instance().insert_ip_to_client(socket, name.toStdString());

            break;
        }

        case LOG_IN: {
            QString name, password;
            stream >> name >> password;

            const bool result = ClientHandler::get_instance().is_password_valid(name.toStdString(), password.toStdString());
            send_cmd_to_client<QString>(socket, STATUS, {"LOGIN", result == 0 ? "FALSE" : "TRUE"});

            if (result == 1)
                ClientHandler::get_instance().insert_ip_to_client(socket, name.toStdString());

            break;
        }

        case ALL_MAILS: {
            auto emails = MailHandler::get_client_mails(ClientHandler::get_instance().get_name_from_client(socket));
            const quint32 emails_count = emails.count();

            send_cmd_to_client<Email>(socket, ALL_MAILS, {emails}, emails_count);
            break;
        }

        case DELETE_A_MAIL: {
            QString mail_hash;
            stream >> mail_hash;

            MailHandler::delete_mail(mail_hash);
            break;
        }
    }
}

template<typename T>
bool Server::send_cmd_to_client(QTcpSocket *client, const Command cmd_type, const QVector<T> &parameters, const quint32 amount) {
    if (client->state() != QTcpSocket::ConnectedState) return false;

    const QByteArray data{pack_data(cmd_type, parameters, amount)};

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