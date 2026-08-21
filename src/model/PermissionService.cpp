#include "model/PermissionService.h"

const QString PermissionService::kMessageNoPermission =
    QStringLiteral("Sie haben keine Berechtigung für diesen Vorgang.");

bool PermissionService::canManageOwnReadingList(UserRole role)
{
    Q_UNUSED(role)
    return true;
}

bool PermissionService::canViewAllReadingLists(UserRole role)
{
    return role == UserRole::KnowledgeManager || role == UserRole::Administrator;
}

bool PermissionService::canApproveForTraining(UserRole role)
{
    return role == UserRole::KnowledgeManager || role == UserRole::Administrator;
}

bool PermissionService::canArchive(UserRole role)
{
    return canApproveForTraining(role);
}

bool PermissionService::canManageUsers(UserRole role)
{
    return role == UserRole::Administrator;
}

bool PermissionService::canSetStatus(UserRole role, ReadingStatus targetStatus)
{
    switch (targetStatus) {
    case ReadingStatus::Noted:
    case ReadingStatus::InProgress:
    case ReadingStatus::Read:
        return true;
    case ReadingStatus::ApprovedForTraining:
        return canApproveForTraining(role);
    case ReadingStatus::Archived:
        return canArchive(role);
    }
    return false;
}

bool PermissionService::canEditEntry(const User &user, const ReadingListEntry &entry)
{
    if (entry.userId() == user.id()) {
        return true;
    }
    if (!canViewAllReadingLists(user.role())) {
        return false;
    }
    return entry.status() == ReadingStatus::Read || entry.status() == ReadingStatus::ApprovedForTraining;
}

bool PermissionService::canDeactivateUser(const User &actingUser, int targetUserId)
{
    if (!canManageUsers(actingUser.role())) {
        return false;
    }
    return targetUserId != actingUser.id();
}
