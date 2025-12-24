//
// Created by Wihy on 12/23/25.
//

#ifndef ROYALEDELIVERYSERVER_COMMANDS_H
#define ROYALEDELIVERYSERVER_COMMANDS_H
#include <QDataStream>

enum Command {
    SIGN_UP,
    LOG_IN,
    STATUS
};

inline QByteArray pack_data(const Command cmd_type, const QStringList& parameters) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);

    stream << static_cast<quint32>(0) << static_cast<quint32>(cmd_type);

    for (const QString& parameter : parameters)
        stream << parameter;

    const quint32 payload_size = data.size() - sizeof(quint32);

    stream.device()->seek(0);
    stream << payload_size;

    return data;
}

#endif //ROYALEDELIVERYSERVER_COMMANDS_H