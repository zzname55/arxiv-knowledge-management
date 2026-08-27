// ---------------------------------------------------------------------------
// PermissionService — MVC layer: MODEL
// Umsetzung from mandatory requirement 1.2. Liegt bewusst im Model: die
// Absicherung greift auch, wenn ein Vorgang an der Oberflaeche vorbei
// triggered wird. Kein Zustand, all Methoden statisch.
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/User.h"
#include "model/UserRole.h"
#include "model/ReadingListEntry.h"
#include "model/ReadingStatus.h"

class PermissionService
{
public:
    static bool canManageOwnReadingList(UserRole role);
    static bool canViewAllReadingLists(UserRole role);
    static bool canApproveForTraining(UserRole role);
    static bool canArchive(UserRole role);
    static bool canManageUsers(UserRole role);
    static bool canSetStatus(UserRole role, ReadingStatus targetStatus);
    static bool canEditEntry(const User &user, const ReadingListEntry &entry);
    static bool canDeactivateUser(const User &actingUser, int targetUserId);

    static const QString kMessageNoPermission;

private:
    PermissionService() = delete;
};
