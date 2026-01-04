#ifndef ROYALEDELIVERYSERVER_SERVER_H
#define ROYALEDELIVERYSERVER_SERVER_H
#include <qobject.h>
#include <QTcpServer>
#include <QTcpSocket>

#include "net/commands.h"

class Server : public QObject {
    Q_OBJECT

public:
    static Server& get_instance() {
        static Server instance;
        return instance;
    }

    Server(const Server&) = delete;
    void operator=(const Server&) = delete;

private:
    std::unordered_map<QString, QString> client_ip_to_name{};
    QTcpServer server{};

    Server();

public slots:
    void new_connection();
    void client_disconnected() const;

    void handle_client_data();
    void handle_error(QAbstractSocket::SocketError socketError) const;

public:
    std::string get_name_from_client(const QTcpSocket *client) const;
    static bool send_cmd_to_client(QTcpSocket *client, Command cmd_type, const QStringList &parameters);
};


#endif //ROYALEDELIVERYSERVER_SERVER_H