/**
 * @file commands.h
 * @brief Defines the network commands and data structures used for communication.
 */

#ifndef ROYALEDELIVERYSERVER_COMMANDS_H
#define ROYALEDELIVERYSERVER_COMMANDS_H
#include <QDataStream>
#include <qmetatype.h>
#include <QIODevice>

/**
 * @enum Command
 * @brief Enumeration of available commands that can be sent between client and server.
 */
enum Command {
    SIGN_UP,       ///< Request to sign up a new user
    LOG_IN,        ///< Request to log in an existing user
    STATUS,        ///< Request/response for status updates
    ALL_MAILS,     ///< Request to retrieve all mails for a user
    DELETE_A_MAIL  ///< Request to delete a specific mail
};

/**
 * @struct Email
 * @brief Represents an email message with its basic components.
 */
struct Email {
    QString to;      ///< Recipient's email address
    QString from;    ///< Sender's email address
    QString subject; ///< Subject line of the email
    QString content; ///< Body content of the email
}; Q_DECLARE_METATYPE(Email)

/**
 * @brief Serializes an Email object into a QDataStream.
 * @param out The output data stream.
 * @param obj The Email object to serialize.
 * @return Reference to the output data stream.
 */
inline QDataStream &operator<<(QDataStream &out, const Email &obj) {
    out << obj.to << obj.from << obj.subject << obj.content;
    return out;
}

/**
 * @brief Deserializes an Email object from a QDataStream.
 * @param in The input data stream.
 * @param obj The Email object to populate.
 * @return Reference to the input data stream.
 */
inline QDataStream &operator>>(QDataStream &in, Email &obj) {
    in >> obj.to >> obj.from >> obj.subject >> obj.content;
    return in;
}

/**
 * @brief Packs a command and its parameters into a QByteArray for network transmission.
 * @tparam T The type of the parameters.
 * @param cmd_type The type of the command being sent.
 * @param parameters A vector containing the parameters for the command.
 * @param amount The number of parameters in the vector.
 * @return A QByteArray containing the packed data, prefixed with the payload size.
 */
template <typename T>
QByteArray pack_data(const Command cmd_type, const QVector<T>& parameters, const quint32 amount) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);

    // Initial size of 0, followed by command type and amount of parameters
    stream << static_cast<quint32>(0) << static_cast<quint32>(cmd_type) << amount;

    // Stream each parameter into the data
    for (const auto& parameter : parameters)
        stream << parameter;

    // Calculate total payload size (excluding the size header itself)
    const quint32 payload_size = data.size() - sizeof(quint32);

    // Go back to the beginning of the stream to write the actual payload size
    stream.device()->seek(0);
    stream << payload_size;

    return data;
}

#endif