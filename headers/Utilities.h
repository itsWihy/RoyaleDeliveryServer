/**
 * @file Utilities.h
 * @brief Header file containing utility functions for the RoyaleDeliveryServer.
 */

#ifndef ROYALEDELIVERYSERVER_UTILITIES_H
#define ROYALEDELIVERYSERVER_UTILITIES_H
#include <QByteArray>
#include <QCryptographicHash>
#include <qstring.h>
#include <string>

/**
 * @brief Hashes a password using the SHA-256 algorithm.
 * @param password The password string to hash.
 * @return The hexadecimal representation of the SHA-256 hash as a std::string.
 */
inline std::string hash(const std::string &password) {
    // Convert the std::string password to QByteArray for Qt's cryptographic functions
    const QByteArray data = QString(password.data()).toUtf8();
    // Calculate the SHA-256 hash and return it as a hex-encoded std::string
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().toStdString();
}

#endif //ROYALEDELIVERYSERVER_UTILITIES_H