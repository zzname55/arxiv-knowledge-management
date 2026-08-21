// ---------------------------------------------------------------------------
// PasswordHasher — MVC layer: MODEL
//
// SHA-256 ueber Salt+Password. Salt verhindert gleiche Hashes bei gleichem
// Password und macht Regenbogentabellen wertlos.
// ---------------------------------------------------------------------------
#pragma once

#include <QString>

class PasswordHasher
{
public:
    /// 16 Zufallsbytes als 32-stellige Hex-Zeichenkette.
    static QString generateSalt();

    /// SHA-256 aus Salt+Password als 64-stellige Hex-Zeichenkette.
    static QString hash(const QString &password, const QString &salt);

    /// Prueft ein Password gegen den gespeicherten Hash. Leeres Password -> immer false.
    static bool verify(const QString &password, const QString &salt, const QString &expectedHash);

    static constexpr int kSaltLaengeInBytes = 16;

private:
    PasswordHasher() = delete;
};
