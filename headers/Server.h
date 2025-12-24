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
    QTcpServer server{};
    QVector<QTcpSocket*> clients{};

    Server();

public slots:
    void new_connection();
    void client_disconnected() const;

    void handle_client_data() const;
    void handle_error(QAbstractSocket::SocketError socketError) const;

public:
    static bool send_cmd_to_client(QTcpSocket *client, Command cmd_type, const QStringList &parameters);
};


#endif //ROYALEDELIVERYSERVER_SERVER_H