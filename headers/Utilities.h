//
// Created by Wihy on 1/3/26.
//

#ifndef ROYALEDELIVERYSERVER_UTILITIES_H
#define ROYALEDELIVERYSERVER_UTILITIES_H
#include <QByteArray>
#include <QCryptographicHash>
#include <qstring.h>
#include <string>

inline std::string hash(const std::string &password) {
    const QByteArray data = QString(password.data()).toUtf8();
    return QString::fromUtf8(QCryptographicHash::hash(data, QCryptographicHash::Sha256)).toStdString();
}

#endif //ROYALEDELIVERYSERVER_UTILITIES_H