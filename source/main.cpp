#include <QCoreApplication>
#include <QTcpServer>
#include <QDir>

#include "../headers/Server.h"
#include "../headers/SMTPServer.h"
#include "../headers/storage/ClientHandler.h"
#include "../headers/storage/MailHandler.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    Server::get_instance();
    SMTPServer::get_instance();
    MailHandler::get_instance();
    ClientHandler::get_instance().write_to_file();

    return a.exec();
}
