#include "model/PasswordHasher.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>

QString PasswordHasher::generateSalt()
{
    QByteArray zufallsbytes(kSaltLaengeInBytes, Qt::Uninitialized);
    QRandomGenerator::system()->generate(zufallsbytes.begin(), zufallsbytes.end());
    return QString::fromLatin1(zufallsbytes.toHex());
}

QString PasswordHasher::hash(const QString &password, const QString &salt)
{
    const QByteArray eingabe = salt.toUtf8() + password.toUtf8();
    return QString::fromLatin1(
        QCryptographicHash::hash(eingabe, QCryptographicHash::Sha256).toHex());
}

bool PasswordHasher::verify(const QString &password, const QString &salt, const QString &expectedHash)
{
    if (password.isEmpty()) {
        return false;
    }
    return hash(password, salt) == expectedHash;
}
