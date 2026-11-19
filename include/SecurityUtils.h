#ifndef SECURITYUTILS_H
#define SECURITYUTILS_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QString>
#include <QUuid>
#include <string>

namespace Security
{

    // Формирует строку вида "<salt_hex>:<sha256_hex>"
    inline std::string hashPassword(const std::string &plainPassword)
    {
        const QByteArray salt = QUuid::createUuid().toByteArray(QUuid::Id128);
        const QByteArray saltHex = salt.toHex();

        QCryptographicHash hasher(QCryptographicHash::Sha256);
        hasher.addData(saltHex);
        hasher.addData(QByteArray::fromStdString(plainPassword));

        const QByteArray digestHex = hasher.result().toHex();
        return (saltHex + ":" + digestHex).toStdString();
    }

    // Проверяет пароль по строке вида "<salt_hex>:<sha256_hex>"
    inline bool verifyPassword(const std::string &plainPassword, const std::string &storedHash)
    {
        const QString encoded = QString::fromStdString(storedHash);
        const int sepIndex = encoded.indexOf(':');
        if (sepIndex <= 0)
        {
            return false;
        }

        const QByteArray saltHex = encoded.left(sepIndex).toLatin1();
        const QByteArray expectedDigestHex = encoded.mid(sepIndex + 1).toLatin1();

        QCryptographicHash hasher(QCryptographicHash::Sha256);
        hasher.addData(saltHex);
        hasher.addData(QByteArray::fromStdString(plainPassword));

        const QByteArray actualDigestHex = hasher.result().toHex();
        return actualDigestHex == expectedDigestHex;
    }

} // namespace Security

#endif