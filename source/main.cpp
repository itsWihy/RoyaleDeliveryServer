#include <QCoreApplication>
#include <QTcpServer>

#include "../headers/Server.h"
#include "../headers/storage/PasswordHandler.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server::get_instance();
    PasswordHandler::get_instance().write_to_file();

    return a.exec();
}
