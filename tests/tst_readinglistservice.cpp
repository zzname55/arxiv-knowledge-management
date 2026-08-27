// ---------------------------------------------------------------------------
// Test: ReadingListService — der betriebliche Prozess als Ganzes
// ---------------------------------------------------------------------------
#include <QtTest>
#include <memory>
#include "model/Database.h"
#include "model/ReadingListService.h"
#include "model/PasswordHasher.h"
#include "model/PermissionService.h"
#include "model/SqliteUserRepository.h"
#include "model/SqliteReadingListRepository.h"
#include "model/SqlitePublicationRepository.h"

class TestLeselisteService : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void aufLeselisteSetzen_legtEintragMitStatusVorgemerktAn();
    void aufLeselisteSetzen_lehntDenZweitenVersuchAb();
    void aufLeselisteSetzen_lehntUnbekannteVeroeffentlichungAb();
    void aufLeselisteSetzen_trenntDieListenZweierBenutzer();

    void prozess_laeuftVomImportBisZumArchiv();
    void statusWechseln_lehntStatussprungAb();
    void statusWechseln_lehntRueckwaertsschrittAb();

    void lesenAbschliessen_speichertBewertungUndNotiz();
    void lesenAbschliessen_lehntBewertungAusserhalbEinsBisFuenfAb();
    void lesenAbschliessen_lehntLeereNotizAb();
    void lesenAbschliessen_speichertNichtsBeiUngueltigerEingabe();
    void completeReading_rejectsWhenNotYetInProgress();

    void statusWechseln_mitarbeiterDarfNichtFreigeben();
    void statusWechseln_mitarbeiterDarfFremdenEintragNichtAendern();
    void alleLeselisten_mitarbeiterErhaeltKeineFremdenEintraege();
    void alleLeselisten_wissensmanagerSiehtAlleEintraege();

    void eigeneListe_liefertNurDieEigenenEintraege();
    void fuerSchulungFreigegebene_enthaeltNurFreigegebene();
    void verwerfen_entferntEinenVorgemerktenEintrag();
    void verwerfen_lehntFremdenEintragAb();

private:
    void legeGrunddatenAn();
    int  legeVeroeffentlichungAn(const QString &arxivId, const QString &title);

    std::unique_ptr<Database>                         m_database;
    std::unique_ptr<SqliteUserRepository>          m_benutzerRepository;
    std::unique_ptr<SqlitePublicationRepository> m_veroeffentlichungRepository;
    std::unique_ptr<SqliteReadingListRepository>         m_leselisteRepository;
    std::unique_ptr<ReadingListService>                  m_service;

    User m_mitarbeiter;
    User m_zweiterMitarbeiter;
    User m_wissensmanager;
    int      m_veroeffentlichungId = 0;
};

void TestLeselisteService::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("test_readingList_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());

    m_benutzerRepository          = std::make_unique<SqliteUserRepository>(*m_database);
    m_veroeffentlichungRepository = std::make_unique<SqlitePublicationRepository>(*m_database);
    m_leselisteRepository         = std::make_unique<SqliteReadingListRepository>(*m_database);

    m_service = std::make_unique<ReadingListService>(*m_leselisteRepository, *m_veroeffentlichungRepository);

    legeGrunddatenAn();
}

void TestLeselisteService::cleanup()
{
    m_service.reset();
    m_leselisteRepository.reset();
    m_veroeffentlichungRepository.reset();
    m_benutzerRepository.reset();
    m_database.reset();
}

void TestLeselisteService::legeGrunddatenAn()
{
    const auto legeBenutzerAn = [this](const QString &username, UserRole role) {
        const QString salt = PasswordHasher::generateSalt();
        User user;
        user.setUsername(username);
        user.setDisplayName(username.toUpper());
        user.setRole(role);
        user.setSalt(salt);
        user.setPasswordHash(PasswordHasher::hash(QStringLiteral("start1234"), salt));
        user.setActive(true);
        m_benutzerRepository->save(user);
        return user;
    };

    m_mitarbeiter        = legeBenutzerAn(QStringLiteral("ma01"), UserRole::Employee);
    m_zweiterMitarbeiter = legeBenutzerAn(QStringLiteral("ma02"), UserRole::Employee);
    m_wissensmanager     = legeBenutzerAn(QStringLiteral("wm01"), UserRole::KnowledgeManager);

    m_veroeffentlichungId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"));
}

int TestLeselisteService::legeVeroeffentlichungAn(const QString &arxivId, const QString &title)
{
    Publication v;
    v.setArxivId(arxivId);
    v.setTitle(title);
    v.setAuthors({ QStringLiteral("A. Autorin") });
    v.setSummary(QStringLiteral("Kurzfassung."));
    v.setArxivCategory(QStringLiteral("cs.LG"));
    v.setDiscipline(Discipline::ComputerScience);
    v.setPublishedAt(QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    v.setUrl(QStringLiteral("https://arxiv.org/abs/%1").arg(arxivId));
    m_veroeffentlichungRepository->save(v);
    return v.id();
}

void TestLeselisteService::aufLeselisteSetzen_legtEintragMitStatusVorgemerktAn()
{
    const OperationResult result = m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId);
    QVERIFY2(result.successful, qPrintable(result.errorMessage));

    const QList<ReadingListEntry> entries = m_service->ownList(m_mitarbeiter);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).status(), ReadingStatus::Noted);
    QVERIFY(entries.at(0).createdAt().isValid());
}

void TestLeselisteService::aufLeselisteSetzen_lehntDenZweitenVersuchAb()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const OperationResult zweiterVersuch = m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId);
    QVERIFY(!zweiterVersuch.successful);
    QCOMPARE(zweiterVersuch.errorMessage, ReadingListService::kMessageAlreadyOnReadingList);
    QCOMPARE(m_service->ownList(m_mitarbeiter).size(), 1);
}

void TestLeselisteService::aufLeselisteSetzen_lehntUnbekannteVeroeffentlichungAb()
{
    const OperationResult result = m_service->addToReadingList(m_mitarbeiter, 9999);
    QVERIFY(!result.successful);
    QVERIFY(!result.errorMessage.isEmpty());
}

void TestLeselisteService::aufLeselisteSetzen_trenntDieListenZweierBenutzer()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);
    QCOMPARE(m_service->ownList(m_mitarbeiter).size(), 1);
    QCOMPARE(m_service->ownList(m_zweiterMitarbeiter).size(), 1);
}

void TestLeselisteService::prozess_laeuftVomImportBisZumArchiv()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();

    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);
    QVERIFY(m_service->completeReading(m_mitarbeiter, entryId, 4, QStringLiteral("Relevant fuer unser Modul.")).successful);
    QVERIFY(m_service->changeStatus(m_wissensmanager, entryId, ReadingStatus::ApprovedForTraining).successful);
    QVERIFY(m_service->changeStatus(m_wissensmanager, entryId, ReadingStatus::Archived).successful);

    const QList<ReadingListEntry> entries = m_service->ownList(m_mitarbeiter);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).status(), ReadingStatus::Archived);
    QCOMPARE(entries.at(0).rating().value(), 4);
}

void TestLeselisteService::statusWechseln_lehntStatussprungAb()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    const OperationResult result = m_service->completeReading(m_mitarbeiter, entryId, 4, QStringLiteral("Note"));
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, ReadingListService::kMessageInvalidTransition);
}

void TestLeselisteService::statusWechseln_lehntRueckwaertsschrittAb()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);
    const OperationResult result = m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::Noted);
    QVERIFY(!result.successful);
}

void TestLeselisteService::lesenAbschliessen_speichertBewertungUndNotiz()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);
    QVERIFY(m_service->completeReading(m_mitarbeiter, entryId, 5, QStringLiteral("Sehr gut.")).successful);

    const ReadingListEntry entry = m_service->ownList(m_mitarbeiter).at(0);
    QCOMPARE(entry.status(), ReadingStatus::Read);
    QCOMPARE(entry.rating().value(), 5);
    QCOMPARE(entry.note(), QStringLiteral("Sehr gut."));
}

void TestLeselisteService::lesenAbschliessen_lehntBewertungAusserhalbEinsBisFuenfAb()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);

    for (const int ungueltig : { 0, 6, -1, 99 }) {
        const OperationResult result = m_service->completeReading(m_mitarbeiter, entryId, ungueltig, QStringLiteral("Note"));
        QVERIFY(!result.successful);
        QCOMPARE(result.errorMessage, ReadingListService::kMessageInvalidRating);
    }
}

void TestLeselisteService::lesenAbschliessen_lehntLeereNotizAb()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);

    const OperationResult result = m_service->completeReading(m_mitarbeiter, entryId, 3, QStringLiteral("   "));
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, ReadingListService::kMessageNoteMissing);
}

void TestLeselisteService::lesenAbschliessen_speichertNichtsBeiUngueltigerEingabe()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);

    m_service->completeReading(m_mitarbeiter, entryId, 9, QStringLiteral("Note"));

    const ReadingListEntry entry = m_service->ownList(m_mitarbeiter).at(0);
    QCOMPARE(entry.status(), ReadingStatus::InProgress);
    QVERIFY(!entry.rating().has_value());
}

void TestLeselisteService::completeReading_rejectsWhenNotYetInProgress()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(!m_service->completeReading(m_mitarbeiter, entryId, 3, QStringLiteral("Note")).successful);
}

void TestLeselisteService::statusWechseln_mitarbeiterDarfNichtFreigeben()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::InProgress).successful);
    QVERIFY(m_service->completeReading(m_mitarbeiter, entryId, 4, QStringLiteral("Note")).successful);

    const OperationResult result = m_service->changeStatus(m_mitarbeiter, entryId, ReadingStatus::ApprovedForTraining);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QCOMPARE(m_service->ownList(m_mitarbeiter).at(0).status(), ReadingStatus::Read);
}

void TestLeselisteService::statusWechseln_mitarbeiterDarfFremdenEintragNichtAendern()
{
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);
    const int fremderEintragId = m_service->ownList(m_zweiterMitarbeiter).at(0).id();

    const OperationResult result = m_service->changeStatus(m_mitarbeiter, fremderEintragId, ReadingStatus::InProgress);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QCOMPARE(m_service->ownList(m_zweiterMitarbeiter).at(0).status(), ReadingStatus::Noted);
}

void TestLeselisteService::alleLeselisten_mitarbeiterErhaeltKeineFremdenEintraege()
{
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);

    OperationResult result;
    const QList<ReadingListEntry> entries = m_service->allReadingLists(m_mitarbeiter, &result);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QVERIFY(entries.isEmpty());
}

void TestLeselisteService::alleLeselisten_wissensmanagerSiehtAlleEintraege()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);

    OperationResult result;
    const QList<ReadingListEntry> entries = m_service->allReadingLists(m_wissensmanager, &result);
    QVERIFY2(result.successful, qPrintable(result.errorMessage));
    QCOMPARE(entries.size(), 2);
    QVERIFY(!entries.at(0).userDisplayName().isEmpty());
    QVERIFY(!entries.at(0).publicationTitle().isEmpty());
}

void TestLeselisteService::eigeneListe_liefertNurDieEigenenEintraege()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);

    const QList<ReadingListEntry> entries = m_service->ownList(m_mitarbeiter);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).userId(), m_mitarbeiter.id());
}

void TestLeselisteService::fuerSchulungFreigegebene_enthaeltNurFreigegebene()
{
    const int zweiteId = legeVeroeffentlichungAn(QStringLiteral("2608.00002"), QStringLiteral("Zweite Arbeit"));

    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int ersterEintragId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->changeStatus(m_mitarbeiter, ersterEintragId, ReadingStatus::InProgress).successful);
    QVERIFY(m_service->completeReading(m_mitarbeiter, ersterEintragId, 5, QStringLiteral("Sehr gut.")).successful);
    QVERIFY(m_service->changeStatus(m_wissensmanager, ersterEintragId, ReadingStatus::ApprovedForTraining).successful);

    QVERIFY(m_service->addToReadingList(m_mitarbeiter, zweiteId).successful);

    const QList<ReadingListEntry> schulungsliste = m_service->approvedForTraining();
    QCOMPARE(schulungsliste.size(), 1);
    QCOMPARE(schulungsliste.at(0).id(), ersterEintragId);
}

void TestLeselisteService::verwerfen_entferntEinenVorgemerktenEintrag()
{
    QVERIFY(m_service->addToReadingList(m_mitarbeiter, m_veroeffentlichungId).successful);
    const int entryId = m_service->ownList(m_mitarbeiter).at(0).id();
    QVERIFY(m_service->discard(m_mitarbeiter, entryId).successful);
    QVERIFY(m_service->ownList(m_mitarbeiter).isEmpty());
}

void TestLeselisteService::verwerfen_lehntFremdenEintragAb()
{
    QVERIFY(m_service->addToReadingList(m_zweiterMitarbeiter, m_veroeffentlichungId).successful);
    const int fremderEintragId = m_service->ownList(m_zweiterMitarbeiter).at(0).id();
    QVERIFY(!m_service->discard(m_mitarbeiter, fremderEintragId).successful);
    QCOMPARE(m_service->ownList(m_zweiterMitarbeiter).size(), 1);
}

QTEST_GUILESS_MAIN(TestLeselisteService)
#include "tst_readinglistservice.moc"
