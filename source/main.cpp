#include <QCoreApplication>
#include <QTcpServer>

#include "../headers/Server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server::get_instance();

    return a.exec();
}
