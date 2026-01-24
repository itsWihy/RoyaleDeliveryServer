//
// Created by Wihy on 12/29/25.
//

#include "../../headers/SMTPServer.h"

#include <iostream>
#include <QSslSocket>

#include "../../headers/Server.h"
#include "../../headers/storage/ClientHandler.h"
#include "../../headers/storage/MailHandler.h"

SMTPServer::SMTPServer() : server() {
    if (!server.listen(QHostAddress::Any, 2500))
        qCritical() << "SMTP Server could not start:" << server.errorString();
    else
        qDebug() << "SMTP Server listening on port 2500";

    connect(&server, &QTcpServer::newConnection, this, &SMTPServer::new_connection);
}

void SMTPServer::handle_mail_body(QSslSocket *client, SmtpSession &session) {
    while (client->canReadLine()) {
        QString currentLine = QString::fromLatin1(client->readLine());

        if (currentLine.trimmed() == ".") {
            client->write("250 OK: Message accepted for delivery\r\n");
            session.state = State::HELO;

            const Email mail = MailHandler::read_from_string(session.total_data);

            const std::string senderName = ClientHandler::get_instance().get_name_from_client(client);
            MailHandler::store_mail(senderName, mail);

            const std::string receiverName = mail.to.split('@').first().toStdString();
            MailHandler::store_mail(receiverName, mail);

            qDebug() << "Email delivered from" << QString::fromStdString(senderName)
                    << "to" << QString::fromStdString(receiverName);

            session.total_data.clear();
            break;
        }

        session.total_data += currentLine;
    }
}

void SMTPServer::new_connection() {
    while (server.hasPendingConnections()) {
        auto client = qobject_cast<QSslSocket *>(server.nextPendingConnection());
        if (!client) continue;

        SmtpSession session;
        session.state = State::UNSET;
        sessions.insert(client, session);

        connect(client, &QSslSocket::disconnected, this, &SMTPServer::client_disconnected);
        connect(client, &QSslSocket::readyRead, this, &SMTPServer::handle_client_data);

        //have to use old syntax else doesnt work
        connect(client, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(handle_ssl_errors(QList<QSslError>)));

        qDebug() << "New connection from:" << client->peerAddress().toString();
        client->write("220 Royale Delivery Service Ready\r\n");
    }
}

void SMTPServer::client_disconnected() {
    if (auto *client = qobject_cast<QSslSocket *>(sender())) {
        sessions.remove(client);
        qDebug() << "Client disconnected, session cleared.";
    }
}

void SMTPServer::handle_client_data() {
    auto *client = qobject_cast<QSslSocket *>(sender());
    if (!client || !sessions.contains(client)) return;

    SmtpSession &session = sessions[client];

    if (session.state == State::DATA) {
        handle_mail_body(client, session);
        return;
    }

    const QString line = QString::fromLatin1(client->readLine()).trimmed();
    if (line.isEmpty()) return;

    qDebug() << "Client" << client->peerAddress().toString() << " sent:" << line;

    if (line.startsWith("STARTTLS", Qt::CaseInsensitive)) {
        client->write("220 Ready to start TLS\r\n");
        client->flush();

        client->setLocalCertificate("../server.crt");
        client->setPrivateKey("../server.key");

        client->startServerEncryption();
        return; //next readyRead will be encrypted. stop here.
    }

    if (line.startsWith("HELO", Qt::CaseInsensitive) || line.startsWith("EHLO", Qt::CaseInsensitive)) {
        session.state = State::HELO;
        if (line.startsWith("EHLO", Qt::CaseInsensitive)) {
            client->write("250-Hello\r\n250-AUTH LOGIN\r\n250-AUTH=LOGIN\r\n250 PIPELINING\r\n");
        } else {
            client->write("250 Hello\r\n");
        }
    } else if (line.startsWith("AUTH LOGIN", Qt::CaseInsensitive)) {
        session.state = State::AUTH_USER;
        client->write("334 VXNlcm5hbWU6\r\n"); //Usernmae
    } else if (session.state == State::AUTH_USER) {
        session.client_name = QByteArray::fromBase64(line.toLatin1());
        session.state = State::AUTH_PASS;
        client->write("334 UGFzc3dvcmQ6\r\n"); //asks for psswrod
    } else if (session.state == State::AUTH_PASS) {
        QString password = QByteArray::fromBase64(line.toLatin1());

        if (ClientHandler::get_instance().is_hashed_password_valid(session.client_name.toStdString(),
                                                                   password.toStdString())) {
            session.is_authenticated = true;
            session.state = State::HELO;
            client->write("235 Authentication successful\r\n");
        } else
            client->write("535 Authentication credentials invalid\r\n");
    } else if (line.startsWith("MAIL FROM:", Qt::CaseInsensitive)) {
        if (!session.is_authenticated) {
            client->write("530 5.7.0 Authentication required\r\n");
            return;
        }

        if (session.state < State::HELO) {
            client->write("503 Bad Sequence: Send HELO/EHLO first\r\n");
            return;
        }
        session.sender = line.mid(10).trimmed().remove('<').remove('>');
        session.state = State::MAIL_FROM;
        client->write("250 Sender ok\r\n");
    } else if (line.startsWith("RCPT TO:", Qt::CaseInsensitive)) {
        if (session.state < State::MAIL_FROM) {
            client->write("503 Bad Sequence: Send MAIL FROM first\r\n");
            return;
        }

        session.recipient = line.mid(8).trimmed().remove('<').remove('>');
        session.state = State::RCPT_TO;
        client->write("250 Recipient ok\r\n");
    } else if (line.compare("DATA", Qt::CaseInsensitive) == 0) {
        if (session.state < State::RCPT_TO) {
            client->write("503 Bad Sequence: Send RCPT TO first\r\n");
            return;
        }

        session.state = State::DATA;
        client->write("354 End with <CRLF>.<CRLF>\r\n");
    } else if (line.compare("QUIT", Qt::CaseInsensitive) == 0) {
        client->write("221 Service closing transmission channel\r\n");
        client->disconnectFromHost();
    } else if (line.compare("RSET", Qt::CaseInsensitive) == 0) {
        session.state = State::HELO;
        session.total_data.clear();
        client->write("250 Ok\r\n");
    } else if (line.compare("NOOP", Qt::CaseInsensitive) == 0) {
        client->write("250 Ok\r\n");
    } else {
        client->write("500 Unknown Command.\r\n");
    }
}

void SMTPServer::handle_error(const QAbstractSocket::SocketError socketError) const {
    const auto *socket = qobject_cast<QSslSocket *>(sender());
    qWarning() << "[Socket Error] Client:" << (socket ? socket->peerAddress().toString() : "Unknown")
            << "Error:" << socketError;
}

void SMTPServer::handle_ssl_errors(const QList<QSslError> &errors) const {
    for (const auto &error : errors) {
        qWarning() << "[SSL Error]:" << error.errorString();
    }

    qobject_cast<QSslSocket*>(sender())->ignoreSslErrors();
}
