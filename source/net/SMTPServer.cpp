/**
 * @file SMTPServer.cpp
 * @brief Implementation of the SMTP server with SSL support and authentication.
 */

#include "../../headers/SMTPServer.h"

#include <iostream>
#include <QSslSocket>

#include "../../headers/Server.h"
#include "../../headers/storage/ClientHandler.h"
#include "../../headers/storage/MailHandler.h"

/**
 * @brief Constructs the SMTP server and starts listening on port 2500.
 */
SMTPServer::SMTPServer() : server() {
    // Start the server on port 2500, which is commonly used for SMTP submission or testing
    if (!server.listen(QHostAddress::Any, 2500))
        qCritical() << "SMTP Server could not start:" << server.errorString();
    else
        qDebug() << "SMTP Server listening on port 2500";

    // Connect the newConnection signal to handle incoming client connections
    connect(&server, &QTcpServer::newConnection, this, &SMTPServer::new_connection);
}

/**
 * @brief Handles the reception of the email body during the DATA state.
 * @param client The SSL socket for the client.
 * @param session The session state for the client.
 */
void SMTPServer::handle_mail_body(QSslSocket *client, SmtpSession &session) {
    while (client->canReadLine()) {
        QString currentLine = QString::fromLatin1(client->readLine());

        // SMTP DATA ends with a single "." on a line
        if (currentLine.trimmed() == ".") {
            client->write("250 OK: Message accepted for delivery\r\n");
            session.state = State::HELO;

            // Parse the accumulated raw mail data into an Email object
            const Email mail = MailHandler::read_from_string(session.total_data);

            // Get the sender's username from the socket mapping
            const std::string senderName = ClientHandler::get_instance().get_name_from_client(client);
            // Store the mail in the sender's outbox/history
            MailHandler::store_mail(senderName, mail);

            // Extract the recipient's username (assuming user@domain format)
            const std::string receiverName = mail.to.split('@').first().toStdString();
            // Store the mail in the recipient's inbox
            MailHandler::store_mail(receiverName, mail);

            qDebug() << "Email delivered from" << QString::fromStdString(senderName)
                    << "to" << QString::fromStdString(receiverName);

            // Clear session data for the next potential mail
            session.total_data.clear();
            break;
        }

        // Accumulate lines into the total_data buffer
        session.total_data += currentLine;
    }
}

/**
 * @brief Handles new incoming connections and initializes the SMTP session.
 */
void SMTPServer::new_connection() {
    while (server.hasPendingConnections()) {
        auto client = qobject_cast<QSslSocket *>(server.nextPendingConnection());
        if (!client) continue;

        // Initialize a new SMTP session for this client
        SmtpSession session;
        session.state = State::UNSET;
        sessions.insert(client, session);

        // Connect relevant signals for data, disconnection, and SSL errors
        connect(client, &QSslSocket::disconnected, this, &SMTPServer::client_disconnected);
        connect(client, &QSslSocket::readyRead, this, &SMTPServer::handle_client_data);

        // SSL errors must be handled specifically; using SIGNAL/SLOT syntax for QList<QSslError>
        connect(client, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(handle_ssl_errors(QList<QSslError>)));

        qDebug() << "New connection from:" << client->peerAddress().toString();
        // Send the initial SMTP greeting
        client->write("220 Royale Delivery Service Ready\r\n");
    }
}

/**
 * @brief Cleans up session data when a client disconnects.
 */
void SMTPServer::client_disconnected() {
    if (auto *client = qobject_cast<QSslSocket *>(sender())) {
        sessions.remove(client);
        qDebug() << "Client disconnected, session cleared.";
    }
}

/**
 * @brief Main SMTP state machine that handles commands from the client.
 */
void SMTPServer::handle_client_data() {
    auto *client = qobject_cast<QSslSocket *>(sender());
    if (!client || !sessions.contains(client)) return;

    SmtpSession &session = sessions[client];

    // If we are in DATA state, divert to handle_mail_body
    if (session.state == State::DATA) {
        handle_mail_body(client, session);
        return;
    }

    // Read a single line and trim whitespace
    const QString line = QString::fromLatin1(client->readLine()).trimmed();
    if (line.isEmpty()) return;

    qDebug() << "Client" << client->peerAddress().toString() << " sent:" << line;

    // STARTTLS command to upgrade the connection to SSL/TLS
    if (line.startsWith("STARTTLS", Qt::CaseInsensitive)) {
        client->write("220 Ready to start TLS\r\n");
        client->flush();

        // Configure SSL certificates (expected to be in the same folder as the executable)
        client->setLocalCertificate("server.crt");
        client->setPrivateKey("server.key");

        // Initiate the SSL handshake
        client->startServerEncryption();
        return; // Next data will be encrypted and trigger readyRead again
    }

    // HELO/EHLO: Identification step
    if (line.startsWith("HELO", Qt::CaseInsensitive) || line.startsWith("EHLO", Qt::CaseInsensitive)) {
        session.state = State::HELO;
        if (line.startsWith("EHLO", Qt::CaseInsensitive)) {
            // EHLO response advertising supported extensions (like AUTH)
            client->write("250-Hello\r\n250-AUTH LOGIN\r\n250-AUTH=LOGIN\r\n250 PIPELINING\r\n");
        } else {
            client->write("250 Hello\r\n");
        }
    } 
    // AUTH LOGIN: Start the authentication process
    else if (line.startsWith("AUTH LOGIN", Qt::CaseInsensitive)) {
        session.state = State::AUTH_USER;
        client->write("334 VXNlcm5hbWU6\r\n"); // "Username:" in base64
    } 
    // Receiving the base64 encoded username
    else if (session.state == State::AUTH_USER) {
        session.client_name = QByteArray::fromBase64(line.toLatin1());
        session.state = State::AUTH_PASS;
        client->write("334 UGFzc3dvcmQ6\r\n"); // "Password:" in base64
    } 
    // Receiving the base64 encoded password and validating it
    else if (session.state == State::AUTH_PASS) {
        QString password = QByteArray::fromBase64(line.toLatin1());

        // Validate credentials against ClientHandler
        if (ClientHandler::get_instance().is_hashed_password_valid(session.client_name.toStdString(),
                                                                   password.toStdString())) {
            session.is_authenticated = true;
            session.state = State::HELO;
            client->write("235 Authentication successful\r\n");
        } else {
            client->write("535 Authentication credentials invalid\r\n");
        }
    } 
    // MAIL FROM: Initiating a mail transaction
    else if (line.startsWith("MAIL FROM:", Qt::CaseInsensitive)) {
        if (!session.is_authenticated) {
            client->write("530 5.7.0 Authentication required\r\n");
            return;
        }

        if (session.state < State::HELO) {
            client->write("503 Bad Sequence: Send HELO/EHLO first\r\n");
            return;
        }
        // Extract sender address from <address>
        session.sender = line.mid(10).trimmed().remove('<').remove('>');
        session.state = State::MAIL_FROM;
        client->write("250 Sender ok\r\n");
    } 
    // RCPT TO: Specifying the recipient
    else if (line.startsWith("RCPT TO:", Qt::CaseInsensitive)) {
        if (session.state < State::MAIL_FROM) {
            client->write("503 Bad Sequence: Send MAIL FROM first\r\n");
            return;
        }

        // Extract recipient address
        session.recipient = line.mid(8).trimmed().remove('<').remove('>');
        session.state = State::RCPT_TO;
        client->write("250 Recipient ok\r\n");
    } 
    // DATA: Signifies the start of the email content
    else if (line.compare("DATA", Qt::CaseInsensitive) == 0) {
        if (session.state < State::RCPT_TO) {
            client->write("503 Bad Sequence: Send RCPT TO first\r\n");
            return;
        }

        session.state = State::DATA;
        client->write("354 End with <CRLF>.<CRLF>\r\n");
    } 
    // QUIT: Terminate the session
    else if (line.compare("QUIT", Qt::CaseInsensitive) == 0) {
        client->write("221 Service closing transmission channel\r\n");
        client->disconnectFromHost();
    } 
    // RSET: Reset the session state
    else if (line.compare("RSET", Qt::CaseInsensitive) == 0) {
        session.state = State::HELO;
        session.total_data.clear();
        client->write("250 Ok\r\n");
    } 
    // NOOP: No operation
    else if (line.compare("NOOP", Qt::CaseInsensitive) == 0) {
        client->write("250 Ok\r\n");
    } 
    // Unknown or unsupported command
    else {
        client->write("500 Unknown Command.\r\n");
    }
}

/**
 * @brief Handles socket errors for SMTP connections.
 * @param socketError The socket error type.
 */
void SMTPServer::handle_error(const QAbstractSocket::SocketError socketError) const {
    const auto *socket = qobject_cast<QSslSocket *>(sender());
    qWarning() << "[Socket Error] Client:" << (socket ? socket->peerAddress().toString() : "Unknown")
            << "Error:" << socketError;
}

/**
 * @brief Handles SSL errors during the handshake or encrypted session.
 * @param errors List of SSL errors encountered.
 */
void SMTPServer::handle_ssl_errors(const QList<QSslError> &errors) const {
    for (const auto &error : errors) {
        qWarning() << "[SSL Error]:" << error.errorString();
    }

    // In a production environment, you should be careful about ignoring SSL errors.
    // For testing with self-signed certificates, this is often necessary.
    qobject_cast<QSslSocket*>(sender())->ignoreSslErrors();
}