/**
 * @file SMTPServer.h
 * @brief Header file for the SMTPServer class, which handles SMTP protocol logic over SSL.
 */

#ifndef ROYALEDELIVERYSERVER_SMTPSERVER_H
#define ROYALEDELIVERYSERVER_SMTPSERVER_H
#include <qobject.h>
#include <QSslSocket>
#include <QTcpServer>

#include "SslServer.h"

/**
 * @enum State
 * @brief Represents the current state of an SMTP session.
 */
enum class State {
    UNSET = 0,    ///< Initial state
    HELO = 1,     ///< HELO/EHLO command received
    AUTH_USER = 2, ///< Waiting for username in authentication
    AUTH_PASS = 3, ///< Waiting for password in authentication
    MAIL_FROM = 4, ///< MAIL FROM command received
    RCPT_TO = 5,   ///< RCPT TO command received
    DATA = 6,     ///< DATA command received, waiting for email body
    QUIT = 7      ///< QUIT command received
};

/**
 * @struct SmtpSession
 * @brief Holds the state and data for an individual SMTP session.
 */
struct SmtpSession {
    State state = State::UNSET; ///< Current SMTP state
    QString total_data;         ///< Accumulated email body data
    QString sender;             ///< Sender email address
    QString recipient;          ///< Recipient email address
    QString client_name;        ///< Name of the connecting client
    bool is_authenticated = false; ///< Authentication status
    bool is_connected = true;    ///< Connection status
};

/**
 * @class SMTPServer
 * @brief Singleton class that implements an SMTP server with SSL support.
 */
class SMTPServer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Gets the singleton instance of the SMTPServer.
     * @return Reference to the SMTPServer instance.
     */
    static SMTPServer &get_instance() {
        static SMTPServer instance;
        return instance;
    }

    SMTPServer(const SMTPServer &) = delete;

    void operator=(const SMTPServer &) = delete;

public slots:
    /**
     * @brief Slot called when a new SSL connection is established.
     */
    void new_connection();

    /**
     * @brief Slot called when an SMTP client disconnects.
     */
    void client_disconnected();

    /**
     * @brief Slot called when data is received from an SMTP client.
     */
    void handle_client_data();

    /**
     * @brief Slot called to handle socket errors.
     * @param socketError The socket error that occurred.
     */
    void handle_error(QAbstractSocket::SocketError socketError) const;

    /**
     * @brief Slot called to handle SSL-specific errors.
     * @param errors List of SSL errors.
     */
    void handle_ssl_errors(const QList<QSslError> &errors) const;

private:
    SslServer server; ///< The underlying SSL server
    QHash<QSslSocket *, SmtpSession> sessions; ///< Map of sockets to their respective SMTP sessions

    /**
     * @brief Private constructor for the Singleton pattern.
     */
    SMTPServer();

    /**
     * @brief Processes the body of an email received during the DATA state.
     * @param client The socket of the client sending the data.
     * @param session The SMTP session associated with the client.
     */
    static void handle_mail_body(QSslSocket *client, SmtpSession &session);
};


#endif //ROYALEDELIVERYSERVER_SMTPSERVER_H