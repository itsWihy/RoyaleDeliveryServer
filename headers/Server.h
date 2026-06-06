/**
 * @file Server.h
 * @brief Header file for the Server class, which manages client connections and basic networking.
 */

#ifndef ROYALEDELIVERYSERVER_SERVER_H
#define ROYALEDELIVERYSERVER_SERVER_H
#include <qobject.h>
#include <QTcpServer>
#include <QTcpSocket>

#include "net/commands.h"

/**
 * @class Server
 * @brief Singleton class that handles the main TCP server logic for client connections.
 */
class Server : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Gets the singleton instance of the Server.
     * @return Reference to the Server instance.
     */
    static Server& get_instance() {
        static Server instance;
        return instance;
    }

    Server(const Server&) = delete;
    void operator=(const Server&) = delete;

private:
    QTcpServer server{};

    /**
     * @brief Private constructor for the Singleton pattern.
     */
    Server();

public slots:
    /**
     * @brief Slot called when a new connection is received.
     */
    void new_connection();

    /**
     * @brief Slot called when a client disconnects.
     */
    void client_disconnected() const;

    /**
     * @brief Slot called when there is data ready to be read from a client.
     */
    void handle_client_data() const;

    /**
     * @brief Slot called to handle socket errors.
     * @param socketError The type of socket error that occurred.
     */
    void handle_error(QAbstractSocket::SocketError socketError) const;

public:
    /**
     * @brief Sends a command with parameters to a specific client.
     * @tparam T The type of the parameters.
     * @param client Pointer to the QTcpSocket representing the client.
     * @param cmd_type The command type to send.
     * @param parameters The vector of parameters to include in the command.
     * @param amount The number of parameters (default is 1).
     * @return True if the command was sent successfully, false otherwise.
     */
    template<class T>
    static bool send_cmd_to_client(QTcpSocket *client, Command cmd_type, const  QVector<T>  &parameters,  quint32 amount=1);
};


#endif //ROYALEDELIVERYSERVER_SERVER_H