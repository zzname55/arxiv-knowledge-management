// ---------------------------------------------------------------------------
// Test: UserRole und User
// ---------------------------------------------------------------------------
#include <QtTest>
#include "model/User.h"
#include "model/UserRole.h"

class TestBenutzerrolle : public QObject
{
    Q_OBJECT

private slots:
    void rolleAlsText_liefertDeutscheBezeichnung();
    void rolleAusText_erkenntAlleRollen();
    void rolleAusText_liefertNichtsBeiUnbekanntemText();
    void umwandlung_istInBeideRichtungenVerlustfrei();
    void alleRollen_enthaeltGenauDreiRollen();
    void user_istStandardmaessigUngueltig();
    void user_haeltDieGesetztenWerte();
    void user_istNachDemAnlegenAktiv();
};

void TestBenutzerrolle::rolleAlsText_liefertDeutscheBezeichnung()
{
    QCOMPARE(roleToText(UserRole::Employee),    QStringLiteral("Employee"));
    QCOMPARE(roleToText(UserRole::KnowledgeManager), QStringLiteral("KnowledgeManager"));
    QCOMPARE(roleToText(UserRole::Administrator),  QStringLiteral("Administrator"));
}

void TestBenutzerrolle::rolleAusText_erkenntAlleRollen()
{
    QCOMPARE(roleFromText(QStringLiteral("Employee")).value(),    UserRole::Employee);
    QCOMPARE(roleFromText(QStringLiteral("KnowledgeManager")).value(), UserRole::KnowledgeManager);
    QCOMPARE(roleFromText(QStringLiteral("Administrator")).value(),  UserRole::Administrator);
}

void TestBenutzerrolle::rolleAusText_liefertNichtsBeiUnbekanntemText()
{
    QVERIFY(!roleFromText(QStringLiteral("Hausmeister")).has_value());
    QVERIFY(!roleFromText(QString()).has_value());
}

void TestBenutzerrolle::umwandlung_istInBeideRichtungenVerlustfrei()
{
    for (const UserRole role : allRoles()) {
        QCOMPARE(roleFromText(roleToText(role)).value(), role);
    }
}

void TestBenutzerrolle::alleRollen_enthaeltGenauDreiRollen()
{
    QCOMPARE(allRoles().size(), 3);
}

void TestBenutzerrolle::user_istStandardmaessigUngueltig()
{
    const User leererBenutzer;
    QVERIFY(!leererBenutzer.isPersisted());
    QVERIFY(leererBenutzer.username().isEmpty());
}

void TestBenutzerrolle::user_haeltDieGesetztenWerte()
{
    User user;
    user.setzeId(7);
    user.setzeBenutzername(QStringLiteral("ma01"));
    user.setzeAnzeigename(QStringLiteral("Max Mustermann"));
    user.setzeRolle(UserRole::Employee);
    user.setzePasswortHash(QStringLiteral("abc"));
    user.setzeSalt(QStringLiteral("def"));
    user.setActive(false);

    QCOMPARE(user.id(), 7);
    QCOMPARE(user.username(), QStringLiteral("ma01"));
    QCOMPARE(user.displayName(), QStringLiteral("Max Mustermann"));
    QCOMPARE(user.role(), UserRole::Employee);
    QCOMPARE(user.passwortHash(), QStringLiteral("abc"));
    QCOMPARE(user.salt(), QStringLiteral("def"));
    QVERIFY(!user.isActive());
    QVERIFY(user.isPersisted());
}

void TestBenutzerrolle::user_istNachDemAnlegenAktiv()
{
    const User user;
    QVERIFY(user.isActive());
}

QTEST_APPLESS_MAIN(TestBenutzerrolle)
#include "tst_userrole.moc"
