// ---------------------------------------------------------------------------
// Test: PermissionService (mandatory requirement 1.2)
// ---------------------------------------------------------------------------
#include <QtTest>
#include "model/ReadingListEntry.h"
#include "model/PermissionService.h"

class TestPermissionService : public QObject
{
    Q_OBJECT

private slots:
    void darfEigeneLeselisteFuehren_giltFuerAlleRollen();
    void darfAlleLeselistenSehen_nurWissensmanagerUndAdministrator();
    void darfAlleLeselistenSehen_nichtFuerMitarbeiter();
    void darfFuerSchulungFreigeben_nurWissensmanagerUndAdministrator();
    void darfFuerSchulungFreigeben_nichtFuerMitarbeiter();
    void darfArchivieren_nichtFuerMitarbeiter();
    void darfBenutzerVerwalten_nurAdministrator();
    void darfBenutzerVerwalten_nichtFuerWissensmanager();
    void employeeMaySetStatusUpToRead();
    void darfStatusSetzen_mitarbeiterNichtDarueberHinaus();
    void darfStatusSetzen_wissensmanagerAuchDieFreigabeschritte();
    void darfEintragBearbeiten_erlaubtDemBesitzer();
    void darfEintragBearbeiten_verbietetFremdeEintraege();
    void darfEintragBearbeiten_erlaubtDemWissensmanagerDieFreigabeschritte();
    void darfBenutzerDeaktivieren_verbietetDasEigeneKonto();
    void darfBenutzerDeaktivieren_erlaubtFremdeKonten();
    void message_istFuerDenBenutzerVerstaendlich();

private:
    static User benutzerMitRolle(UserRole role, int id = 1);
    static ReadingListEntry eintragVon(int userId, ReadingStatus status);
};

User TestPermissionService::benutzerMitRolle(UserRole role, int id)
{
    User user;
    user.setId(id);
    user.setUsername(QStringLiteral("nutzer%1").arg(id));
    user.setRole(role);
    user.setActive(true);
    return user;
}

ReadingListEntry TestPermissionService::eintragVon(int userId, ReadingStatus status)
{
    ReadingListEntry entry;
    entry.setId(100);
    entry.setUserId(userId);
    entry.setPublicationId(200);
    entry.setStatus(status);
    return entry;
}

void TestPermissionService::darfEigeneLeselisteFuehren_giltFuerAlleRollen()
{
    for (const UserRole role : allRoles()) {
        QVERIFY(PermissionService::canManageOwnReadingList(role));
    }
}

void TestPermissionService::darfAlleLeselistenSehen_nurWissensmanagerUndAdministrator()
{
    QVERIFY(PermissionService::canViewAllReadingLists(UserRole::KnowledgeManager));
    QVERIFY(PermissionService::canViewAllReadingLists(UserRole::Administrator));
}

void TestPermissionService::darfAlleLeselistenSehen_nichtFuerMitarbeiter()
{
    QVERIFY(!PermissionService::canViewAllReadingLists(UserRole::Employee));
}

void TestPermissionService::darfFuerSchulungFreigeben_nurWissensmanagerUndAdministrator()
{
    QVERIFY(PermissionService::canApproveForTraining(UserRole::KnowledgeManager));
    QVERIFY(PermissionService::canApproveForTraining(UserRole::Administrator));
}

void TestPermissionService::darfFuerSchulungFreigeben_nichtFuerMitarbeiter()
{
    QVERIFY(!PermissionService::canApproveForTraining(UserRole::Employee));
}

void TestPermissionService::darfArchivieren_nichtFuerMitarbeiter()
{
    QVERIFY(!PermissionService::canArchive(UserRole::Employee));
    QVERIFY(PermissionService::canArchive(UserRole::KnowledgeManager));
}

void TestPermissionService::darfBenutzerVerwalten_nurAdministrator()
{
    QVERIFY(PermissionService::canManageUsers(UserRole::Administrator));
}

void TestPermissionService::darfBenutzerVerwalten_nichtFuerWissensmanager()
{
    QVERIFY(!PermissionService::canManageUsers(UserRole::KnowledgeManager));
    QVERIFY(!PermissionService::canManageUsers(UserRole::Employee));
}

void TestPermissionService::employeeMaySetStatusUpToRead()
{
    QVERIFY(PermissionService::canSetStatus(UserRole::Employee, ReadingStatus::InProgress));
    QVERIFY(PermissionService::canSetStatus(UserRole::Employee, ReadingStatus::Read));
}

void TestPermissionService::darfStatusSetzen_mitarbeiterNichtDarueberHinaus()
{
    QVERIFY(!PermissionService::canSetStatus(UserRole::Employee, ReadingStatus::ApprovedForTraining));
    QVERIFY(!PermissionService::canSetStatus(UserRole::Employee, ReadingStatus::Archived));
}

void TestPermissionService::darfStatusSetzen_wissensmanagerAuchDieFreigabeschritte()
{
    QVERIFY(PermissionService::canSetStatus(UserRole::KnowledgeManager, ReadingStatus::ApprovedForTraining));
    QVERIFY(PermissionService::canSetStatus(UserRole::KnowledgeManager, ReadingStatus::Archived));
}

void TestPermissionService::darfEintragBearbeiten_erlaubtDemBesitzer()
{
    const User mitarbeiter = benutzerMitRolle(UserRole::Employee, 1);
    const ReadingListEntry eigener = eintragVon(1, ReadingStatus::Noted);
    QVERIFY(PermissionService::canEditEntry(mitarbeiter, eigener));
}

void TestPermissionService::darfEintragBearbeiten_verbietetFremdeEintraege()
{
    const User mitarbeiter = benutzerMitRolle(UserRole::Employee, 1);
    const ReadingListEntry fremder = eintragVon(2, ReadingStatus::Noted);
    QVERIFY(!PermissionService::canEditEntry(mitarbeiter, fremder));
}

void TestPermissionService::darfEintragBearbeiten_erlaubtDemWissensmanagerDieFreigabeschritte()
{
    const User wissensmanager = benutzerMitRolle(UserRole::KnowledgeManager, 9);
    QVERIFY(PermissionService::canEditEntry(wissensmanager, eintragVon(1, ReadingStatus::Read)));
    QVERIFY(PermissionService::canEditEntry(wissensmanager, eintragVon(1, ReadingStatus::ApprovedForTraining)));
    QVERIFY(!PermissionService::canEditEntry(wissensmanager, eintragVon(1, ReadingStatus::InProgress)));
    QVERIFY(!PermissionService::canEditEntry(wissensmanager, eintragVon(1, ReadingStatus::Noted)));
}

void TestPermissionService::darfBenutzerDeaktivieren_verbietetDasEigeneKonto()
{
    const User administrator = benutzerMitRolle(UserRole::Administrator, 3);
    QVERIFY(!PermissionService::canDeactivateUser(administrator, 3));
}

void TestPermissionService::darfBenutzerDeaktivieren_erlaubtFremdeKonten()
{
    const User administrator = benutzerMitRolle(UserRole::Administrator, 3);
    QVERIFY(PermissionService::canDeactivateUser(administrator, 7));

    const User mitarbeiter = benutzerMitRolle(UserRole::Employee, 1);
    QVERIFY(!PermissionService::canDeactivateUser(mitarbeiter, 7));
}

void TestPermissionService::message_istFuerDenBenutzerVerstaendlich()
{
    const QString message = PermissionService::kMessageNoPermission;
    QVERIFY(!message.isEmpty());
    QVERIFY(message.contains(QStringLiteral("permission")));
}

QTEST_APPLESS_MAIN(TestPermissionService)
#include "tst_permissionservice.moc"
