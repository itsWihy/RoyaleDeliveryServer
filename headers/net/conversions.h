#ifndef ROYALEDELIVERYCLIENT_CONVERSIONS_H
#define ROYALEDELIVERYCLIENT_CONVERSIONS_H
#include <QByteArray>
#include <QDataStream>
#include <qglobal.h>
#include <QIODevice>

inline qint32 byte_array_to_int(QByteArray source) {
    qint32 size;
    QDataStream temp{&source, QIODevice::WriteOnly};
    temp >> size;
    return size;
}

inline QByteArray int_to_byte_array(qint32 number) {
    QByteArray size;
    QDataStream temp{&size, QIODevice::WriteOnly};
    temp << number;
    return size;
}

#endif //ROYALEDELIVERYCLIENT_CONVERSIONS_H