/**
 * @file main.cpp
 * @brief Entry point for the RoayleDeliveryServer application.
 */

#include <QCoreApplication>
#include <QTcpServer>
#include <QDir>

#include "../headers/Server.h"
#include "../headers/SMTPServer.h"
#include "../headers/storage/ClientHandler.h"
#include "../headers/storage/MailHandler.h"

/**
 * @brief Main function that initializes the server and its components.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Execution status code.
 */
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Set the current working directory to the application's directory
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    // Initialize Singleton instances
    // This starts the main TCP server
    Server::get_instance();
    // This starts the SMTP server (with SSL)
    SMTPServer::get_instance();
    // Initialize the mail handler
    MailHandler::get_instance();
    // Initialize the client handler and ensure the storage file exists/is updated
    ClientHandler::get_instance().write_to_file();

    // Start the Qt event loop
    return a.exec();
}