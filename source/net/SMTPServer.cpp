//
// Created by Wihy on 12/29/25.
//

#include "../../headers/SMTPServer.h"

#include <iostream>
#include <QTcpSocket>

#include "../../headers/Server.h"
#include "../../headers/storage/ClientHandler.h"
#include "../../headers/storage/MailHandler.h"

SMTPServer::SMTPServer() : server(this), state(State::UNSET) {
    server.listen(QHostAddress::Any, 2500);
    connect(&server, &QTcpServer::newConnection, this, &SMTPServer::new_connection);
}

void SMTPServer::handle_mail_body(QTcpSocket *client) {
    QString totalData = "";

    while (client->canReadLine()) {
        QString currentLine = QString::fromLatin1(client->readLine());

        if (currentLine.trimmed() == ".") {
            client->write("250 OK: Message accepted for delivery\r\n");
            state = State::HELO;


            //Store mail on SENDER
            MailHandler::store_mail(ClientHandler::get_instance().get_name_from_client(client), Email{totalData, totalData, totalData, totalData});

            //Store mail for the RECEIVER if exists
            Email flipped = MailHandler::read_from_string(totalData);

            const auto temp = flipped.to;
            flipped.to = flipped.from;
            flipped.from = temp;

            MailHandler::store_mail(flipped.to.toStdString(), flipped);
//TODO: TEST ABOVE!!!!!!!!!!!!!!
            qDebug() << "RECEIVED: " << totalData;

            break;
        }

        totalData += currentLine;
    }

}

bool SMTPServer::new_connection() {
    QTcpSocket *client = server.nextPendingConnection();

    connect(client, &QTcpSocket::disconnected, this, &SMTPServer::client_disconnected);
    connect(client, &QTcpSocket::readyRead, this, &SMTPServer::handle_client_data);
    connect(client, &QAbstractSocket::errorOccurred, this, &SMTPServer::handle_error);

    keep_connection.try_emplace(client, client_status{true, false});

    qDebug() << "SEND SERVICe READY TO CLIENT" << client->write("220 Service Ready.\r\n");

    return false;
}

void SMTPServer::client_disconnected() {
    auto *client = qobject_cast<QTcpSocket *>(sender());

    keep_connection.at(client).is_connected = false;
    keep_connection.at(client).has_started_transaction = false;

    qDebug("Client disconnected buddy");
}

void SMTPServer::handle_client_data() {
    auto *client = qobject_cast<QTcpSocket *>(sender());

    if (state == State::DATA) {
        handle_mail_body(client);
        return;
    }

    QString line = QString::fromLatin1(client->readLine()).trimmed();
    if (line.isEmpty()) return;

    qDebug() << "Received command:" << line;

    if (line.startsWith("HELO", Qt::CaseInsensitive)) {
        state = State::HELO;
        client->write("250 Hello\r\n");
    } else if (line.startsWith("EHLO", Qt::CaseInsensitive)) {
        state = State::HELO;
        client->write("250-Hello\r\n250-AUTH LOGIN\r\n250-AUTH=LOGIN\r\n250 PIPELINING\r\n");
    } else if (line.startsWith("MAIL FROM:", Qt::CaseInsensitive)) {
        if (state < State::HELO) {
            client->write("503 Bad Sequence\r\n");
            return;
        }
        client->write("250 Sender ok\r\n");
        state = State::MAIL_FROM;
    } else if (line.startsWith("RCPT TO:", Qt::CaseInsensitive)) {
        if (state < State::MAIL_FROM) {
            client->write("503 Bad Sequence\r\n");
            return;
        }

        state = State::RCPT_TO;
        client->write("250 Recipient ok\r\n");
    } else if (line.startsWith("VRFY ", Qt::CaseInsensitive)) {
        client->write("252 Verifiyng isn't possible, but accepting.\r\n");
    } else if (line.compare("NOOP", Qt::CaseInsensitive) == 0) {
        client->write("250 Ok\r\n");
    } else if (line.compare("RSET", Qt::CaseInsensitive) == 0) {
        state = State::HELO;
        keep_connection.at(client).has_started_transaction = false;

        client->write("250 Ok\r\n");
    } else if (line.compare("QUIT", Qt::CaseInsensitive) == 0) {
        keep_connection.at(client).is_connected = false;

        client->write("221 Service closing transmission channel\r\n");
        client->disconnectFromHost();
    } else if (line.compare("DATA", Qt::CaseInsensitive) == 0) {
        if (state < State::RCPT_TO) {
            client->write("503 Bad Sequence\r\n");
            return;
        }

        client->write("354 End with <CRLF>.<CRLF>\r\n");
        state = State::DATA;
    } else {
        client->write("500 Unknown Command.\r\n");
    }
}


void SMTPServer::handle_error(const QAbstractSocket::SocketError socketError) const {
    const auto *socket = qobject_cast<QTcpSocket *>(sender());
    const char *errorMessage = nullptr;

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
