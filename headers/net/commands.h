#ifndef ROYALEDELIVERYSERVER_COMMANDS_H
#define ROYALEDELIVERYSERVER_COMMANDS_H
#include <QDataStream>

enum Command {
    SIGN_UP,
    LOG_IN,
    STATUS,
    ALL_MAILS,
    DELETE_A_MAIL
};

struct Email {
    QString to;
    QString from;
    QString subject;
    QString content;
}; Q_DECLARE_METATYPE(Email)

inline QDataStream &operator<<(QDataStream &out, const Email &obj) {
    out << obj.to << obj.from << obj.subject << obj.content;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, Email &obj) {
    in >> obj.to >> obj.from >> obj.subject >> obj.content;
    return in;
}


template <typename T>
QByteArray pack_data(const Command cmd_type, const QVector<T>& parameters, const quint32 amount) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);

    stream << static_cast<quint32>(0) << static_cast<quint32>(cmd_type) << amount;

    for (const auto& parameter : parameters)
        stream << parameter;

    const quint32 payload_size = data.size() - sizeof(quint32);

    stream.device()->seek(0);
    stream << payload_size;

    return data;
}

#endif
