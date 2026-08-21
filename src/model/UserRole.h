// ---------------------------------------------------------------------------
// UserRole — MVC layer: MODEL
//
// Die drei Rollen der Anwendung (mandatory requirement 1.2).
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QList>
#include <optional>

enum class UserRole {
    Employee,
    KnowledgeManager,
    Administrator
};

/// Deutsche Bezeichnung fuer Anzeige und Database.
QString roleToText(UserRole role);

/// Umkehrung von roleToText().
std::optional<UserRole> roleFromText(const QString &text);

/// Alle Rollen, aufsteigend nach Berechtigungsumfang.
QList<UserRole> allRoles();
