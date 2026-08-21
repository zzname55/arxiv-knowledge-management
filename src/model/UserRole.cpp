#include "model/UserRole.h"

QString roleToText(UserRole role)
{
    switch (role) {
    case UserRole::Employee:    return QStringLiteral("Employee");
    case UserRole::KnowledgeManager: return QStringLiteral("KnowledgeManager");
    case UserRole::Administrator:  return QStringLiteral("Administrator");
    }
    return QString();
}

std::optional<UserRole> roleFromText(const QString &text)
{
    for (const UserRole role : allRoles()) {
        if (roleToText(role) == text) {
            return role;
        }
    }
    return std::nullopt;
}

QList<UserRole> allRoles()
{
    return { UserRole::Employee,
             UserRole::KnowledgeManager,
             UserRole::Administrator };
}
