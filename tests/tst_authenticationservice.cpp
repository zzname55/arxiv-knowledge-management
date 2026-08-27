// ---------------------------------------------------------------------------
// Test: AuthenticationService (US-01, negative Testfaelle)
// ---------------------------------------------------------------------------
#include <QtTest>
#include "FakeUserRepository.h"
#include "model/AuthenticationService.h"
#include "model/PasswordHasher.h"

class TestAuthentifizierungsService : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void anmelden_gelingtMitRichtigenZugangsdaten();
    void anmelden_setztDenAngemeldetenBenutzer();
    void anmelden_scheitertBeiFalschemPasswort();
    void anmelden_scheitertBeiUnbekanntemBenutzernamen();
    void anmelden_meldungVerraetNichtWelchesFeldFalschWar();
    void anmelden_scheitertBeiLeeremBenutzernamen();
    void anmelden_scheitertBeiLeeremPasswort();
    void anmelden_scheitertBeiDeaktiviertemKonto();
    void anmelden_scheiterndeAnmeldungLaesstSitzungUnangetastet();
    void logout_beendetDieSitzung();
    void istAngemeldet_istVorDerAnmeldungFalsch();

private:
    void legeTestbenutzerAn(int id, const QString &username, const QString &password, UserRole role, bool aktiv);

    FakeUserRepository                     m_repository;
    std::unique_ptr<AuthenticationService> m_service;
};

void TestAuthentifizierungsService::init()
{
    m_repository = FakeUserRepository();
    m_service    = std::make_unique<AuthenticationService>(m_repository);

    legeTestbenutzerAn(1, QStringLiteral("ma01"),    QStringLiteral("start1234"), UserRole::Employee,   true);
    legeTestbenutzerAn(2, QStringLiteral("admin01"), QStringLiteral("admin1234"), UserRole::Administrator, true);
    legeTestbenutzerAn(3, QStringLiteral("alt01"),   QStringLiteral("start1234"), UserRole::Employee,   false);
}

void TestAuthentifizierungsService::legeTestbenutzerAn(int id, const QString &username, const QString &password, UserRole role, bool aktiv)
{
    const QString salt = PasswordHasher::generateSalt();

    User user;
    user.setId(id);
    user.setUsername(username);
    user.setDisplayName(username.toUpper());
    user.setRole(role);
    user.setSalt(salt);
    user.setPasswordHash(PasswordHasher::hash(password, salt));
    user.setActive(aktiv);

    m_repository.fuegeEinFuerTest(user);
}

void TestAuthentifizierungsService::anmelden_gelingtMitRichtigenZugangsdaten()
{
    const AnmeldeErgebnis result = m_service->login(QStringLiteral("ma01"), QStringLiteral("start1234"));
    QVERIFY2(result.successful, qPrintable(result.errorMessage));
    QCOMPARE(result.user.username(), QStringLiteral("ma01"));
    QCOMPARE(result.user.role(), UserRole::Employee);
    QVERIFY(result.errorMessage.isEmpty());
}

void TestAuthentifizierungsService::anmelden_setztDenAngemeldetenBenutzer()
{
    m_service->login(QStringLiteral("admin01"), QStringLiteral("admin1234"));
    QVERIFY(m_service->isLoggedIn());
    QCOMPARE(m_service->currentUser()->role(), UserRole::Administrator);
}

void TestAuthentifizierungsService::anmelden_scheitertBeiFalschemPasswort()
{
    const AnmeldeErgebnis result = m_service->login(QStringLiteral("ma01"), QStringLiteral("falsch999"));
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, QStringLiteral("Username or password is incorrect."));
    QVERIFY(!m_service->isLoggedIn());
}

void TestAuthentifizierungsService::anmelden_scheitertBeiUnbekanntemBenutzernamen()
{
    const AnmeldeErgebnis result = m_service->login(QStringLiteral("gibtesnicht"), QStringLiteral("start1234"));
    QVERIFY(!result.successful);
    QVERIFY(!m_service->isLoggedIn());
}

void TestAuthentifizierungsService::anmelden_meldungVerraetNichtWelchesFeldFalschWar()
{
    const AnmeldeErgebnis a = m_service->login(QStringLiteral("ma01"), QStringLiteral("falsch999"));
    const AnmeldeErgebnis b = m_service->login(QStringLiteral("gibtesnicht"), QStringLiteral("start1234"));
    QCOMPARE(a.errorMessage, b.errorMessage);
}

void TestAuthentifizierungsService::anmelden_scheitertBeiLeeremBenutzernamen()
{
    const AnmeldeErgebnis result = m_service->login(QString(), QStringLiteral("start1234"));
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, QStringLiteral("Please enter a username and password."));
}

void TestAuthentifizierungsService::anmelden_scheitertBeiLeeremPasswort()
{
    const AnmeldeErgebnis result = m_service->login(QStringLiteral("ma01"), QString());
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, QStringLiteral("Please enter a username and password."));
}

void TestAuthentifizierungsService::anmelden_scheitertBeiDeaktiviertemKonto()
{
    const AnmeldeErgebnis result = m_service->login(QStringLiteral("alt01"), QStringLiteral("start1234"));
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, QStringLiteral("This account is deactivated."));
    QVERIFY(!m_service->isLoggedIn());
}

void TestAuthentifizierungsService::anmelden_scheiterndeAnmeldungLaesstSitzungUnangetastet()
{
    m_service->login(QStringLiteral("ma01"), QStringLiteral("start1234"));
    m_service->login(QStringLiteral("admin01"), QStringLiteral("falsch"));
    QVERIFY(m_service->isLoggedIn());
    QCOMPARE(m_service->currentUser()->username(), QStringLiteral("ma01"));
}

void TestAuthentifizierungsService::logout_beendetDieSitzung()
{
    m_service->login(QStringLiteral("ma01"), QStringLiteral("start1234"));
    m_service->logout();
    QVERIFY(!m_service->isLoggedIn());
}

void TestAuthentifizierungsService::istAngemeldet_istVorDerAnmeldungFalsch()
{
    QVERIFY(!m_service->isLoggedIn());
}

QTEST_APPLESS_MAIN(TestAuthentifizierungsService)
#include "tst_authenticationservice.moc"
