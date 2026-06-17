/**
 * @file Server.cpp
 * @brief Implementation of the Server class for handling client connections and commands.
 */

#include "../../headers/Server.h"

#include <iostream>

#include "../../headers/net/commands.h"
#include "../../headers/storage/ClientHandler.h"
#include "../../headers/storage/MailHandler.h"

/**
 * @brief Constructs a Server object and starts listening for connections on port 5004.
 */
Server::Server() : server(this) {
    // Start listening on all available network interfaces on port 5004
    server.listen(QHostAddress::Any, 5004);
    // Connect the newConnection signal to our new_connection slot
    connect(&server, &QTcpServer::newConnection, this, &Server::new_connection);
}

/**
 * @brief Handles a new incoming connection.
 */
void Server::new_connection() {
    // Get the next pending connection as a QTcpSocket
    QTcpSocket *client = server.nextPendingConnection();

    const QString ipAddress = client->peerAddress().toString();
    const int port = client->peerPort();

    std::cout << "New connection from " << ipAddress.toStdString() << ":" << port << std::endl;

    // Connect socket signals to their respective handler slots
    connect(client, &QTcpSocket::disconnected, this, &Server::client_disconnected);
    connect(client, &QTcpSocket::readyRead, this, &Server::handle_client_data);
    connect(client, &QAbstractSocket::errorOccurred, this, &Server::handle_error);

    qDebug() << "Socket connected from " + ipAddress + ":" + QString::number(port);
}

/**
 * @brief Handles client disconnection.
 */
void Server::client_disconnected() const {
    const QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    const QString client_ip_addr = client->peerAddress().toString();
    const int client_port = client->peerPort();

    qDebug() << "Socket disconnected from " + client_ip_addr + ":" + QString::number(client_port);
}

/**
 * @brief Processes incoming data from a client and executes the corresponding command.
 */
void Server::handle_client_data() const {
    auto *socket = qobject_cast<QTcpSocket *>(sender());

    // Read all data from the socket into a QDataStream
    QDataStream stream(socket->readAll());
    stream.setVersion(QDataStream::Qt_5_15);

    // Use transactions to ensure we have read enough data
    stream.startTransaction();

    quint32 length, cmd_type;
    stream >> length >> cmd_type;

    // If we haven't received the full header, wait for more data
    if (!stream.commitTransaction()) return;

    // Handle different command types
    switch (cmd_type) {
        case SIGN_UP: {
            QString name, password;
            stream >> name >> password;

            std::cout << "[SIGNUP] Attempt for user: " << name.toStdString() << std::endl;

            // Attempt to register a new user
            const bool result = ClientHandler::get_instance().insert_new_client(name.toStdString(), password.toStdString());

            std::cout << "[SIGNUP] Result for " << name.toStdString() << ": " << (result ? "SUCCESS" : "FAILED") << std::endl;

            // Send back the status of the signup operation
            send_cmd_to_client<QString>(socket, STATUS, {"SIGNUP", result == 0 ? "FALSE" : "TRUE"});

            // If successful, map the current socket to this username
            if (result == 1)
                ClientHandler::get_instance().insert_ip_to_client(socket, name.toStdString());

            break;
        }

        case LOG_IN: {
            QString name, password;
            stream >> name >> password;

            std::cout << "[LOGIN] Attempt for user: " << name.toStdString() << std::endl;

            // Validate user credentials
            const bool result = ClientHandler::get_instance().is_password_valid(name.toStdString(), password.toStdString());

            std::cout << "[LOGIN] Result for " << name.toStdString() << ": " << (result ? "SUCCESS" : "FAILED") << std::endl;

            // Send back the status of the login operation
            send_cmd_to_client<QString>(socket, STATUS, {"LOGIN", result == 0 ? "FALSE" : "TRUE"});

            // If successful, map the current socket to this username
            if (result == 1)
                ClientHandler::get_instance().insert_ip_to_client(socket, name.toStdString());

            break;
        }

        case ALL_MAILS: {
            // Retrieve all mails for the user associated with this socket
            auto emails = MailHandler::get_client_mails(ClientHandler::get_instance().get_name_from_client(socket));
            const quint32 emails_count = emails.count();

            // Send the list of emails back to the client
            send_cmd_to_client<Email>(socket, ALL_MAILS, {emails}, emails_count);
            break;
        }

        case DELETE_A_MAIL: {
            QString mail_hash;
            stream >> mail_hash;

            // Delete the specified mail
            MailHandler::delete_mail(mail_hash);
            break;
        }
    }
}

/**
 * @brief Template function to pack and send a command to a client.
 * @tparam T Type of parameters to be sent.
 * @param client The socket of the target client.
 * @param cmd_type The command type being sent.
 * @param parameters Vector of parameters.
 * @param amount Number of parameters.
 * @return True if the write operation was initiated successfully.
 */
template<typename T>
bool Server::send_cmd_to_client(QTcpSocket *client, const Command cmd_type, const QVector<T> &parameters, const quint32 amount) {
    if (client->state() != QTcpSocket::ConnectedState) return false;

    // Pack the command data using the utility function from commands.h
    const QByteArray data{pack_data(cmd_type, parameters, amount)};

    return client->write(data) >= 0;
}

/**
 * @brief Handles socket errors and logs them to stderr.
 * @param socketError The type of socket error.
 */
void Server::handle_error(const QAbstractSocket::SocketError socketError) const {
    const auto *socket = qobject_cast<QTcpSocket *>(sender());
    const char* errorMessage = nullptr;

    // Map common socket errors to user-friendly messages
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
