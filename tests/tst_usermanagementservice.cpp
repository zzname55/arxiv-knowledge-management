// ---------------------------------------------------------------------------
// Test: UserManagementService (US-08)
// ---------------------------------------------------------------------------
#include <QtTest>
#include <memory>
#include "model/UserManagementService.h"
#include "model/Database.h"
#include "model/PasswordHasher.h"
#include "model/PermissionService.h"
#include "model/SqliteUserRepository.h"

class TestBenutzerverwaltungsService : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void benutzerAnlegen_legtKontoMitGehashtemPasswortAn();
    void benutzerAnlegen_lehntVergebenenBenutzernamenAb();
    void benutzerAnlegen_lehntZuKurzesPasswortAb();
    void benutzerAnlegen_lehntLeeredPflichtangabenAb();
    void benutzerAnlegen_lehntMitarbeiterAb();
    void benutzerAnlegen_lehntWissensmanagerAb();
    void benutzerAnlegen_vergibtJeKontoEinenEigenenSalt();

    void rolleAendern_setztDieNeueRolle();
    void rolleAendern_lehntUnbekanntesKontoAb();
    void rolleAendern_lehntMitarbeiterAb();

    void kontoDeaktivieren_sperrtDasKonto();
    void kontoDeaktivieren_lehntDasEigeneKontoAb();
    void kontoDeaktivieren_lehntMitarbeiterAb();
    void kontoAktivieren_gibtEinKontoWiederFrei();

    void alleBenutzer_lieferAdministratorDieVollstaendigeListe();
    void alleBenutzer_lehntMitarbeiterAb();

private:
    std::unique_ptr<Database>                  m_database;
    std::unique_ptr<SqliteUserRepository>   m_repository;
    std::unique_ptr<UserManagementService> m_service;

    User m_administrator;
    User m_wissensmanager;
    User m_mitarbeiter;
};

void TestBenutzerverwaltungsService::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("test_bv_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());

    m_repository = std::make_unique<SqliteUserRepository>(*m_database);
    m_service    = std::make_unique<UserManagementService>(*m_repository);

    QVERIFY(m_repository->createDefaultUsers());

    m_mitarbeiter    = m_repository->findByUsername(QStringLiteral("ma01")).value();
    m_wissensmanager = m_repository->findByUsername(QStringLiteral("wm01")).value();
    m_administrator  = m_repository->findByUsername(QStringLiteral("admin01")).value();
}

void TestBenutzerverwaltungsService::cleanup()
{
    m_service.reset();
    m_repository.reset();
    m_database.reset();
}

void TestBenutzerverwaltungsService::benutzerAnlegen_legtKontoMitGehashtemPasswortAn()
{
    const OperationResult result = m_service->createUser(m_administrator, QStringLiteral("ma03"), QStringLiteral("Mia Meier"),
                                                                  QStringLiteral("geheim1234"), UserRole::Employee);
    QVERIFY2(result.successful, qPrintable(result.errorMessage));

    const auto angelegt = m_repository->findByUsername(QStringLiteral("ma03"));
    QVERIFY(angelegt.has_value());
    QCOMPARE(angelegt->displayName(), QStringLiteral("Mia Meier"));
    QCOMPARE(angelegt->role(), UserRole::Employee);
    QVERIFY(angelegt->isActive());
    QVERIFY(!angelegt->passwortHash().contains(QStringLiteral("geheim1234")));
    QVERIFY(PasswordHasher::verify(QStringLiteral("geheim1234"), angelegt->salt(), angelegt->passwortHash()));
}

void TestBenutzerverwaltungsService::benutzerAnlegen_lehntVergebenenBenutzernamenAb()
{
    const OperationResult result = m_service->createUser(m_administrator, QStringLiteral("ma01"), QStringLiteral("Doppelgänger"),
                                                                  QStringLiteral("geheim1234"), UserRole::Employee);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, UserManagementService::kMessageUsernameTaken);
    QCOMPARE(m_repository->all().size(), 3);
}

void TestBenutzerverwaltungsService::benutzerAnlegen_lehntZuKurzesPasswortAb()
{
    const OperationResult result = m_service->createUser(m_administrator, QStringLiteral("ma03"), QStringLiteral("Mia Meier"),
                                                                  QStringLiteral("kurz"), UserRole::Employee);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, UserManagementService::kMessagePasswordTooShort);
    QVERIFY(!m_repository->usernameExists(QStringLiteral("ma03")));
}

void TestBenutzerverwaltungsService::benutzerAnlegen_lehntLeeredPflichtangabenAb()
{
    QVERIFY(!m_service->createUser(m_administrator, QString(), QStringLiteral("Mia Meier"),
                                        QStringLiteral("geheim1234"), UserRole::Employee).successful);
    QVERIFY(!m_service->createUser(m_administrator, QStringLiteral("ma03"), QStringLiteral("   "),
                                        QStringLiteral("geheim1234"), UserRole::Employee).successful);
    QCOMPARE(m_repository->all().size(), 3);
}

void TestBenutzerverwaltungsService::benutzerAnlegen_lehntMitarbeiterAb()
{
    const OperationResult result = m_service->createUser(m_mitarbeiter, QStringLiteral("ma03"), QStringLiteral("Mia Meier"),
                                                                  QStringLiteral("geheim1234"), UserRole::Administrator);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QCOMPARE(m_repository->all().size(), 3);
}

void TestBenutzerverwaltungsService::benutzerAnlegen_lehntWissensmanagerAb()
{
    QVERIFY(!m_service->createUser(m_wissensmanager, QStringLiteral("ma03"), QStringLiteral("Mia Meier"),
                                        QStringLiteral("geheim1234"), UserRole::Employee).successful);
}

void TestBenutzerverwaltungsService::benutzerAnlegen_vergibtJeKontoEinenEigenenSalt()
{
    QVERIFY(m_service->createUser(m_administrator, QStringLiteral("ma03"), QStringLiteral("Mia Meier"),
                                       QStringLiteral("gleichesPasswort"), UserRole::Employee).successful);
    QVERIFY(m_service->createUser(m_administrator, QStringLiteral("ma04"), QStringLiteral("Tom Tester"),
                                       QStringLiteral("gleichesPasswort"), UserRole::Employee).successful);

    const auto ersterBenutzer  = m_repository->findByUsername(QStringLiteral("ma03")).value();
    const auto zweiterBenutzer = m_repository->findByUsername(QStringLiteral("ma04")).value();
    QVERIFY(ersterBenutzer.salt() != zweiterBenutzer.salt());
    QVERIFY(ersterBenutzer.passwortHash() != zweiterBenutzer.passwortHash());
}

void TestBenutzerverwaltungsService::rolleAendern_setztDieNeueRolle()
{
    const OperationResult result = m_service->changeRole(m_administrator, m_mitarbeiter.id(), UserRole::KnowledgeManager);
    QVERIFY2(result.successful, qPrintable(result.errorMessage));
    QCOMPARE(m_repository->findById(m_mitarbeiter.id())->role(), UserRole::KnowledgeManager);
}

void TestBenutzerverwaltungsService::rolleAendern_lehntUnbekanntesKontoAb()
{
    const OperationResult result = m_service->changeRole(m_administrator, 9999, UserRole::KnowledgeManager);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, UserManagementService::kMessageUserUnknown);
}

void TestBenutzerverwaltungsService::rolleAendern_lehntMitarbeiterAb()
{
    const OperationResult result = m_service->changeRole(m_mitarbeiter, m_mitarbeiter.id(), UserRole::Administrator);
    QVERIFY(!result.successful);
    QCOMPARE(m_repository->findById(m_mitarbeiter.id())->role(), UserRole::Employee);
}

void TestBenutzerverwaltungsService::kontoDeaktivieren_sperrtDasKonto()
{
    const OperationResult result = m_service->deactivateAccount(m_administrator, m_mitarbeiter.id());
    QVERIFY2(result.successful, qPrintable(result.errorMessage));
    QVERIFY(!m_repository->findById(m_mitarbeiter.id())->isActive());
}

void TestBenutzerverwaltungsService::kontoDeaktivieren_lehntDasEigeneKontoAb()
{
    const OperationResult result = m_service->deactivateAccount(m_administrator, m_administrator.id());
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QVERIFY(m_repository->findById(m_administrator.id())->isActive());
}

void TestBenutzerverwaltungsService::kontoDeaktivieren_lehntMitarbeiterAb()
{
    const OperationResult result = m_service->deactivateAccount(m_mitarbeiter, m_wissensmanager.id());
    QVERIFY(!result.successful);
    QVERIFY(m_repository->findById(m_wissensmanager.id())->isActive());
}

void TestBenutzerverwaltungsService::kontoAktivieren_gibtEinKontoWiederFrei()
{
    QVERIFY(m_service->deactivateAccount(m_administrator, m_mitarbeiter.id()).successful);
    QVERIFY(m_service->activateAccount(m_administrator, m_mitarbeiter.id()).successful);
    QVERIFY(m_repository->findById(m_mitarbeiter.id())->isActive());
}

void TestBenutzerverwaltungsService::alleBenutzer_lieferAdministratorDieVollstaendigeListe()
{
    OperationResult result;
    const QList<User> userList = m_service->allUsers(m_administrator, &result);
    QVERIFY2(result.successful, qPrintable(result.errorMessage));
    QCOMPARE(userList.size(), 3);
}

void TestBenutzerverwaltungsService::alleBenutzer_lehntMitarbeiterAb()
{
    OperationResult result;
    const QList<User> userList = m_service->allUsers(m_mitarbeiter, &result);
    QVERIFY(!result.successful);
    QVERIFY(userList.isEmpty());
}

QTEST_GUILESS_MAIN(TestBenutzerverwaltungsService)
#include "tst_usermanagementservice.moc"
