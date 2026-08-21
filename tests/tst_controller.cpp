// ---------------------------------------------------------------------------
// Test: die vier Controller (mandatory requirement 1.4)
// Views sind durch Testdoppel ersetzt -- kein einziges widget noetig.
// ---------------------------------------------------------------------------
#include <QtTest>
#include <memory>

#include "FakeViews.h"

#include "controller/LoginController.h"
#include "controller/UserManagementController.h"
#include "controller/ReadingListController.h"
#include "controller/PublicationController.h"

#include "model/ArxivClient.h"
#include "model/AuthenticationService.h"
#include "model/UserManagementService.h"
#include "model/Database.h"
#include "model/ReadingListService.h"
#include "model/PermissionService.h"
#include "model/SqliteUserRepository.h"
#include "model/SqliteReadingListRepository.h"
#include "model/SqlitePublicationRepository.h"

class TestController : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void anmelden_meldetErfolgAnDieAnsicht();
    void anmelden_reichtDieFehlermeldungDesModelsUnveraendertWeiter();
    void anmelden_loeschtDieFehleranzeigeVorJedemVersuch();
    void anmelden_oeffnetBeiFehlschlagNichtDasHauptfenster();
    void logout_beendetDieSitzungUndMeldetEsDerAnsicht();

    void publications_zeigtDieGespeichertenArbeiten();
    void publications_filtertNachDisziplin();
    void publications_meldetTrefferzahlUndGesamtzahl();
    void publications_zeigtHinweisWennDerFilterNichtsFindet();
    void abruf_speichertDieEmpfangenenArbeitenUndZeigtSieAn();
    void abruf_legtBekannteArbeitenNichtDoppeltAn();
    void abruf_reichtDenNetzwerkfehlerAnDieAnsichtWeiter();
    void abruf_schaltetDieLadeanzeigeWiederAus();

    void readingList_setztEineArbeitAufDieListeUndAktualisiert();
    void readingList_meldetDenZweitenVersuchAlsFehler();
    void readingList_fuehrtDenStatuswechselAus();
    void readingList_reichtDieAblehnungDesModelsWeiter();
    void readingList_zeigtDemMitarbeiterKeineFremdenEintraege();
    void readingList_zeigtDemWissensmanagerAlleEintraege();

    void userManagement_legtEinKontoAnUndSetztDasFormularZurueck();
    void userManagement_reichtDieFehlermeldungWeiterUndBehaeltDasFormular();
    void userManagement_weistEinenMitarbeiterAb();

private:
    void meldeAn(const QString &username);
    int  legeVeroeffentlichungAn(const QString &arxivId, const QString &title, Discipline discipline);
    static Publication baueVeroeffentlichung(const QString &arxivId, const QString &title, Discipline discipline);

    std::unique_ptr<Database>                         m_database;
    std::unique_ptr<SqliteUserRepository>          m_benutzerRepository;
    std::unique_ptr<SqlitePublicationRepository> m_veroeffentlichungRepository;
    std::unique_ptr<SqliteReadingListRepository>         m_leselisteRepository;

    std::unique_ptr<AuthenticationService>  m_authentication;
    std::unique_ptr<ReadingListService>           m_readingListService;
    std::unique_ptr<UserManagementService> m_benutzerverwaltungsService;
    std::unique_ptr<ArxivClient>                m_arxivClient;

    FakeLoginViewContract             m_anmeldeAnsicht;
    FakePublicationViewContract  m_veroeffentlichungsAnsicht;
    FakeReadingListViewContract          m_leselistenAnsicht;
    FakeUserManagementViewContract m_benutzerverwaltungsAnsicht;

    std::unique_ptr<LoginController>            m_anmeldeController;
    std::unique_ptr<PublicationController>  m_veroeffentlichungController;
    std::unique_ptr<ReadingListController>          m_leselisteController;
    std::unique_ptr<UserManagementController> m_benutzerverwaltungController;
};

void TestController::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("test_ctrl_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());

    m_benutzerRepository          = std::make_unique<SqliteUserRepository>(*m_database);
    m_veroeffentlichungRepository = std::make_unique<SqlitePublicationRepository>(*m_database);
    m_leselisteRepository         = std::make_unique<SqliteReadingListRepository>(*m_database);
    QVERIFY(m_benutzerRepository->createDefaultUsers());

    m_authentication = std::make_unique<AuthenticationService>(*m_benutzerRepository);
    m_readingListService  = std::make_unique<ReadingListService>(*m_leselisteRepository, *m_veroeffentlichungRepository);
    m_benutzerverwaltungsService = std::make_unique<UserManagementService>(*m_benutzerRepository);
    m_arxivClient = std::make_unique<ArxivClient>();

    m_anmeldeAnsicht             = FakeLoginViewContract();
    m_veroeffentlichungsAnsicht  = FakePublicationViewContract();
    m_leselistenAnsicht          = FakeReadingListViewContract();
    m_benutzerverwaltungsAnsicht = FakeUserManagementViewContract();

    m_anmeldeController = std::make_unique<LoginController>(*m_authentication, m_anmeldeAnsicht);
    m_veroeffentlichungController = std::make_unique<PublicationController>(*m_arxivClient, *m_veroeffentlichungRepository, m_veroeffentlichungsAnsicht);
    m_leselisteController = std::make_unique<ReadingListController>(*m_readingListService, *m_authentication, m_leselistenAnsicht);
    m_benutzerverwaltungController = std::make_unique<UserManagementController>(*m_benutzerverwaltungsService, *m_authentication, m_benutzerverwaltungsAnsicht);
}

void TestController::cleanup()
{
    m_benutzerverwaltungController.reset();
    m_leselisteController.reset();
    m_veroeffentlichungController.reset();
    m_anmeldeController.reset();
    m_arxivClient.reset();
    m_benutzerverwaltungsService.reset();
    m_readingListService.reset();
    m_authentication.reset();
    m_leselisteRepository.reset();
    m_veroeffentlichungRepository.reset();
    m_benutzerRepository.reset();
    m_database.reset();
}

void TestController::meldeAn(const QString &username)
{
    m_anmeldeAnsicht.eingegebenerBenutzername = username;
    m_anmeldeAnsicht.eingegebenesPasswort     = SqliteUserRepository::kDefaultPassword;
    m_anmeldeController->loginRequested();
    QVERIFY(m_authentication->isLoggedIn());
}

Publication TestController::baueVeroeffentlichung(const QString &arxivId, const QString &title, Discipline discipline)
{
    Publication v;
    v.setzeArxivId(arxivId);
    v.setzeTitel(title);
    v.setzeAutoren({ QStringLiteral("A. Autorin") });
    v.setzeZusammenfassung(QStringLiteral("Kurzfassung."));
    v.setzeArxivKategorie(QStringLiteral("cs.LG"));
    v.setzeDisziplin(discipline);
    v.setzeVeroeffentlichtAm(QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    v.setzeUrl(QStringLiteral("https://arxiv.org/abs/%1").arg(arxivId));
    return v;
}

int TestController::legeVeroeffentlichungAn(const QString &arxivId, const QString &title, Discipline discipline)
{
    Publication v = baueVeroeffentlichung(arxivId, title, discipline);
    m_veroeffentlichungRepository->save(v);
    return v.id();
}

void TestController::anmelden_meldetErfolgAnDieAnsicht()
{
    m_anmeldeAnsicht.eingegebenerBenutzername = QStringLiteral("ma01");
    m_anmeldeAnsicht.eingegebenesPasswort     = SqliteUserRepository::kDefaultPassword;
    m_anmeldeController->loginRequested();

    QVERIFY(m_anmeldeAnsicht.anmeldungAbgeschlossenAufgerufen);
    QCOMPARE(m_anmeldeAnsicht.currentUser.username(), QStringLiteral("ma01"));
    QCOMPARE(m_anmeldeAnsicht.currentUser.role(), UserRole::Employee);
    QVERIFY(m_anmeldeAnsicht.angezeigterFehler.isEmpty());
}

void TestController::anmelden_reichtDieFehlermeldungDesModelsUnveraendertWeiter()
{
    m_anmeldeAnsicht.eingegebenerBenutzername = QStringLiteral("ma01");
    m_anmeldeAnsicht.eingegebenesPasswort     = QStringLiteral("falsch999");
    m_anmeldeController->loginRequested();

    QCOMPARE(m_anmeldeAnsicht.angezeigterFehler, AuthenticationService::kMessageInvalidCredentials);
}

void TestController::anmelden_loeschtDieFehleranzeigeVorJedemVersuch()
{
    m_anmeldeAnsicht.eingegebenerBenutzername = QStringLiteral("ma01");
    m_anmeldeAnsicht.eingegebenesPasswort     = QStringLiteral("falsch999");
    m_anmeldeController->loginRequested();

    const int vorher = m_anmeldeAnsicht.anzahlFehlerGeloescht;
    m_anmeldeAnsicht.eingegebenesPasswort = SqliteUserRepository::kDefaultPassword;
    m_anmeldeController->loginRequested();

    QVERIFY(m_anmeldeAnsicht.anzahlFehlerGeloescht > vorher);
}

void TestController::anmelden_oeffnetBeiFehlschlagNichtDasHauptfenster()
{
    m_anmeldeAnsicht.eingegebenerBenutzername = QStringLiteral("ma01");
    m_anmeldeAnsicht.eingegebenesPasswort     = QStringLiteral("falsch999");
    m_anmeldeController->loginRequested();

    QVERIFY(!m_anmeldeAnsicht.anmeldungAbgeschlossenAufgerufen);
    QVERIFY(!m_authentication->isLoggedIn());
}

void TestController::logout_beendetDieSitzungUndMeldetEsDerAnsicht()
{
    meldeAn(QStringLiteral("ma01"));
    m_anmeldeController->logoutRequested();
    QVERIFY(!m_authentication->isLoggedIn());
    QVERIFY(m_anmeldeAnsicht.abmeldungAufgerufen);
}

void TestController::publications_zeigtDieGespeichertenArbeiten()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.size(), 1);
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.at(0).title(), QStringLiteral("ComputerScience-Arbeit"));
}

void TestController::publications_filtertNachDisziplin()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);
    legeVeroeffentlichungAn(QStringLiteral("2608.00002"), QStringLiteral("Mathematics-Arbeit"), Discipline::Mathematics);
    m_veroeffentlichungController->disciplineSelected(Discipline::Mathematics);
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.size(), 1);
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.at(0).title(), QStringLiteral("Mathematics-Arbeit"));
}

void TestController::publications_meldetTrefferzahlUndGesamtzahl()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);
    legeVeroeffentlichungAn(QStringLiteral("2608.00002"), QStringLiteral("Mathematics-Arbeit"), Discipline::Mathematics);
    m_veroeffentlichungController->disciplineSelected(Discipline::Mathematics);
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteTrefferzahl, 1);
    QCOMPARE(m_veroeffentlichungsAnsicht.gesamtzahl, 2);
}

void TestController::publications_zeigtHinweisWennDerFilterNichtsFindet()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->disciplineSelected(Discipline::QuantitativeBiology);
    QVERIFY(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.isEmpty());
    QVERIFY(!m_veroeffentlichungsAnsicht.angezeigterHinweis.isEmpty());
}

void TestController::abruf_speichertDieEmpfangenenArbeitenUndZeigtSieAn()
{
    const QList<Publication> vonArxiv = { baueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Neue Arbeit"), Discipline::ComputerScience) };
    m_veroeffentlichungController->publicationsReceived(vonArxiv);
    QCOMPARE(m_veroeffentlichungRepository->count(), 1);
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigteVeroeffentlichungen.size(), 1);
}

void TestController::abruf_legtBekannteArbeitenNichtDoppeltAn()
{
    const QList<Publication> vonArxiv = { baueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Neue Arbeit"), Discipline::ComputerScience) };
    m_veroeffentlichungController->publicationsReceived(vonArxiv);
    m_veroeffentlichungController->publicationsReceived(vonArxiv);
    QCOMPARE(m_veroeffentlichungRepository->count(), 1);
}

void TestController::abruf_reichtDenNetzwerkfehlerAnDieAnsichtWeiter()
{
    m_veroeffentlichungController->fetchFailed(QStringLiteral("arXiv ist nicht erreichbar."));
    QCOMPARE(m_veroeffentlichungsAnsicht.angezeigterFehler, QStringLiteral("arXiv ist nicht erreichbar."));
    QVERIFY(!m_veroeffentlichungsAnsicht.ladeanzeigeSichtbar);
}

void TestController::abruf_schaltetDieLadeanzeigeWiederAus()
{
    m_veroeffentlichungController->fetchRequested();
    QCOMPARE(m_veroeffentlichungsAnsicht.anzahlLadeanzeigeAn, 1);
    m_veroeffentlichungController->publicationsReceived({});
    QVERIFY(!m_veroeffentlichungsAnsicht.ladeanzeigeSichtbar);
}

void TestController::readingList_setztEineArbeitAufDieListeUndAktualisiert()
{
    meldeAn(QStringLiteral("ma01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);

    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.size(), 1);
    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.at(0).status(), ReadingStatus::Noted);
    QVERIFY(m_leselistenAnsicht.angezeigterFehler.isEmpty());
    QVERIFY(!m_leselistenAnsicht.angezeigteMeldung.isEmpty());
}

void TestController::readingList_meldetDenZweitenVersuchAlsFehler()
{
    meldeAn(QStringLiteral("ma01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);
    m_leselisteController->addToReadingList(publicationId);

    QCOMPARE(m_leselistenAnsicht.angezeigterFehler, ReadingListService::kMessageAlreadyOnReadingList);
    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.size(), 1);
}

void TestController::readingList_fuehrtDenStatuswechselAus()
{
    meldeAn(QStringLiteral("ma01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);
    const int entryId = m_leselistenAnsicht.angezeigteEintraege.at(0).id();
    m_leselisteController->startReading(entryId);
    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.at(0).status(), ReadingStatus::InProgress);
}

void TestController::readingList_reichtDieAblehnungDesModelsWeiter()
{
    meldeAn(QStringLiteral("ma01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);
    const int entryId = m_leselistenAnsicht.angezeigteEintraege.at(0).id();
    m_leselisteController->startReading(entryId);
    m_leselisteController->completeReading(entryId, 4, QStringLiteral("Note"));

    m_leselisteController->approveForTraining(entryId);

    QCOMPARE(m_leselistenAnsicht.angezeigterFehler, PermissionService::kMessageNoPermission);
    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.at(0).status(), ReadingStatus::Gelesen);
}

void TestController::readingList_zeigtDemMitarbeiterKeineFremdenEintraege()
{
    meldeAn(QStringLiteral("wm01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);

    m_anmeldeController->logoutRequested();
    meldeAn(QStringLiteral("ma01"));

    m_leselisteController->refreshAllLists();

    QVERIFY(m_leselistenAnsicht.angezeigteEintraege.isEmpty());
    QCOMPARE(m_leselistenAnsicht.angezeigterFehler, PermissionService::kMessageNoPermission);
}

void TestController::readingList_zeigtDemWissensmanagerAlleEintraege()
{
    meldeAn(QStringLiteral("ma01"));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Arbeit"), Discipline::ComputerScience);
    m_leselisteController->addToReadingList(publicationId);

    m_anmeldeController->logoutRequested();
    meldeAn(QStringLiteral("wm01"));

    m_leselisteController->refreshAllLists();

    QCOMPARE(m_leselistenAnsicht.angezeigteEintraege.size(), 1);
    QVERIFY(m_leselistenAnsicht.angezeigterFehler.isEmpty());
}

void TestController::userManagement_legtEinKontoAnUndSetztDasFormularZurueck()
{
    meldeAn(QStringLiteral("admin01"));
    m_benutzerverwaltungController->createUser(QStringLiteral("ma03"), QStringLiteral("Mia Meier"), QStringLiteral("geheim1234"), UserRole::Employee);

    QVERIFY(m_benutzerverwaltungsAnsicht.angezeigterFehler.isEmpty());
    QCOMPARE(m_benutzerverwaltungsAnsicht.angezeigteBenutzer.size(), 4);
    QCOMPARE(m_benutzerverwaltungsAnsicht.anzahlFormularZurueckgesetzt, 1);
}

void TestController::userManagement_reichtDieFehlermeldungWeiterUndBehaeltDasFormular()
{
    meldeAn(QStringLiteral("admin01"));
    m_benutzerverwaltungController->createUser(QStringLiteral("ma03"), QStringLiteral("Mia Meier"), QStringLiteral("kurz"), UserRole::Employee);

    QCOMPARE(m_benutzerverwaltungsAnsicht.angezeigterFehler, UserManagementService::kMessagePasswordTooShort);
    QCOMPARE(m_benutzerverwaltungsAnsicht.anzahlFormularZurueckgesetzt, 0);
}

void TestController::userManagement_weistEinenMitarbeiterAb()
{
    meldeAn(QStringLiteral("ma01"));
    m_benutzerverwaltungController->createUser(QStringLiteral("ma03"), QStringLiteral("Mia Meier"), QStringLiteral("geheim1234"), UserRole::Administrator);

    QCOMPARE(m_benutzerverwaltungsAnsicht.angezeigterFehler, PermissionService::kMessageNoPermission);
    QCOMPARE(m_benutzerRepository->all().size(), 3);
}

QTEST_GUILESS_MAIN(TestController)
#include "tst_controller.moc"
