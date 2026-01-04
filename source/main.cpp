#include <QCoreApplication>
#include <QTcpServer>

#include "../headers/Server.h"
#include "../headers/SMTPServer.h"
#include "../headers/storage/ClientHandler.h"
#include "../headers/storage/MailHandler.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server::get_instance();
    SMTPServer::get_instance();
    ClientHandler::get_instance().write_to_file();
    MailHandler::get_instance().store_mail("f", "F");

    return a.exec();
}
