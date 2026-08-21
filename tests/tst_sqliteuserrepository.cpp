// ---------------------------------------------------------------------------
// Test: SqliteUserRepository
// ---------------------------------------------------------------------------
#include <QtTest>
#include <memory>
#include "model/Database.h"
#include "model/PasswordHasher.h"
#include "model/SqliteUserRepository.h"

class TestSqliteBenutzerRepository : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void speichere_vergibtEineIdBeimAnlegen();
    void speichere_aktualisiertEinenBestehendenBenutzer();
    void speichere_lehntDoppeltenBenutzernamenAb();
    void findeNachBenutzername_liefertAlleFelderZurueck();
    void findeNachBenutzername_liefertNichtsBeiUnbekanntemNamen();
    void findeNachId_liefertDenRichtigenBenutzer();
    void alle_enthaeltAuchDeaktivierteKonten();
    void existiertBenutzername_erkenntVergebeneNamen();
    void role_ueberstehtDenWegDurchDieDatenbank();
    void deaktivierung_bleibtNachDemNeuenLesenErhalten();
    void legeStandardbenutzerAn_erzeugtDreiKonten();
    void legeStandardbenutzerAn_istMehrfachAufrufbar();

private:
    std::unique_ptr<Database>                m_database;
    std::unique_ptr<SqliteUserRepository> m_repository;

    static User newUser(const QString &username, UserRole role = UserRole::Employee);
};

User TestSqliteBenutzerRepository::newUser(const QString &username, UserRole role)
{
    const QString salt = PasswordHasher::generateSalt();
    User user;
    user.setzeBenutzername(username);
    user.setzeAnzeigename(QStringLiteral("DisplayName %1").arg(username));
    user.setzeSalt(salt);
    user.setzePasswortHash(PasswordHasher::hash(QStringLiteral("start1234"), salt));
    user.setzeRolle(role);
    user.setActive(true);
    return user;
}

void TestSqliteBenutzerRepository::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("test_repo_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());
    m_repository = std::make_unique<SqliteUserRepository>(*m_database);
}

void TestSqliteBenutzerRepository::cleanup()
{
    m_repository.reset();
    m_database.reset();
}

void TestSqliteBenutzerRepository::speichere_vergibtEineIdBeimAnlegen()
{
    User user = newUser(QStringLiteral("ma01"));
    QVERIFY(!user.isPersisted());
    QVERIFY2(m_repository->save(user), qPrintable(m_repository->lastError()));
    QVERIFY(user.isPersisted());
    QVERIFY(user.id() > 0);
}

void TestSqliteBenutzerRepository::speichere_aktualisiertEinenBestehendenBenutzer()
{
    User user = newUser(QStringLiteral("ma01"));
    QVERIFY(m_repository->save(user));
    const int vergebeneId = user.id();

    user.setzeAnzeigename(QStringLiteral("Neuer Name"));
    user.setzeRolle(UserRole::KnowledgeManager);
    QVERIFY(m_repository->save(user));

    QCOMPARE(user.id(), vergebeneId);
    QCOMPARE(m_repository->all().size(), 1);

    const auto gelesen = m_repository->findById(vergebeneId);
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->displayName(), QStringLiteral("Neuer Name"));
    QCOMPARE(gelesen->role(), UserRole::KnowledgeManager);
}

void TestSqliteBenutzerRepository::speichere_lehntDoppeltenBenutzernamenAb()
{
    User erster = newUser(QStringLiteral("ma01"));
    QVERIFY(m_repository->save(erster));

    User zweiter = newUser(QStringLiteral("ma01"));
    QVERIFY(!m_repository->save(zweiter));
    QVERIFY(!m_repository->lastError().isEmpty());
    QCOMPARE(m_repository->all().size(), 1);
}

void TestSqliteBenutzerRepository::findeNachBenutzername_liefertAlleFelderZurueck()
{
    User user = newUser(QStringLiteral("wm01"), UserRole::KnowledgeManager);
    QVERIFY(m_repository->save(user));

    const auto gelesen = m_repository->findByUsername(QStringLiteral("wm01"));
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->id(), user.id());
    QCOMPARE(gelesen->username(), user.username());
    QCOMPARE(gelesen->role(), UserRole::KnowledgeManager);
    QCOMPARE(gelesen->isActive(), true);
}

void TestSqliteBenutzerRepository::findeNachBenutzername_liefertNichtsBeiUnbekanntemNamen()
{
    QVERIFY(!m_repository->findByUsername(QStringLiteral("gibtesnicht")).has_value());
}

void TestSqliteBenutzerRepository::findeNachId_liefertDenRichtigenBenutzer()
{
    User erster = newUser(QStringLiteral("ma01"));
    User zweiter = newUser(QStringLiteral("ma02"));
    QVERIFY(m_repository->save(erster));
    QVERIFY(m_repository->save(zweiter));

    const auto gelesen = m_repository->findById(zweiter.id());
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->username(), QStringLiteral("ma02"));
    QVERIFY(!m_repository->findById(9999).has_value());
}

void TestSqliteBenutzerRepository::alle_enthaeltAuchDeaktivierteKonten()
{
    User aktiv = newUser(QStringLiteral("ma01"));
    User gesperrt = newUser(QStringLiteral("alt01"));
    gesperrt.setActive(false);

    QVERIFY(m_repository->save(aktiv));
    QVERIFY(m_repository->save(gesperrt));
    QCOMPARE(m_repository->all().size(), 2);
}

void TestSqliteBenutzerRepository::existiertBenutzername_erkenntVergebeneNamen()
{
    User user = newUser(QStringLiteral("ma01"));
    QVERIFY(m_repository->save(user));
    QVERIFY(m_repository->usernameExists(QStringLiteral("ma01")));
    QVERIFY(!m_repository->usernameExists(QStringLiteral("ma02")));
}

void TestSqliteBenutzerRepository::role_ueberstehtDenWegDurchDieDatenbank()
{
    for (const UserRole role : allRoles()) {
        User user = newUser(QStringLiteral("nutzer_%1").arg(roleToText(role)), role);
        QVERIFY(m_repository->save(user));
        const auto gelesen = m_repository->findById(user.id());
        QVERIFY(gelesen.has_value());
        QCOMPARE(gelesen->role(), role);
    }
}

void TestSqliteBenutzerRepository::deaktivierung_bleibtNachDemNeuenLesenErhalten()
{
    User user = newUser(QStringLiteral("ma01"));
    QVERIFY(m_repository->save(user));
    user.setActive(false);
    QVERIFY(m_repository->save(user));

    const auto gelesen = m_repository->findByUsername(QStringLiteral("ma01"));
    QVERIFY(gelesen.has_value());
    QVERIFY(!gelesen->isActive());
}

void TestSqliteBenutzerRepository::legeStandardbenutzerAn_erzeugtDreiKonten()
{
    QVERIFY(m_repository->createDefaultUsers());
    QCOMPARE(m_repository->all().size(), 3);

    const auto mitarbeiter = m_repository->findByUsername(QStringLiteral("ma01"));
    QVERIFY(mitarbeiter.has_value());
    QCOMPARE(mitarbeiter->role(), UserRole::Employee);

    const auto wissensmanager = m_repository->findByUsername(QStringLiteral("wm01"));
    QVERIFY(wissensmanager.has_value());
    QCOMPARE(wissensmanager->role(), UserRole::KnowledgeManager);

    const auto administrator = m_repository->findByUsername(QStringLiteral("admin01"));
    QVERIFY(administrator.has_value());
    QCOMPARE(administrator->role(), UserRole::Administrator);

    QVERIFY(PasswordHasher::verify(SqliteUserRepository::kDefaultPassword, mitarbeiter->salt(), mitarbeiter->passwortHash()));
}

void TestSqliteBenutzerRepository::legeStandardbenutzerAn_istMehrfachAufrufbar()
{
    QVERIFY(m_repository->createDefaultUsers());
    QVERIFY(m_repository->createDefaultUsers());
    QCOMPARE(m_repository->all().size(), 3);
}

QTEST_GUILESS_MAIN(TestSqliteBenutzerRepository)
#include "tst_sqliteuserrepository.moc"
