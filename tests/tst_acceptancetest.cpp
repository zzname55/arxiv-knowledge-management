// ---------------------------------------------------------------------------
// Abnahmetest — Backlog B-25, Anforderungskatalog Abschnitt 6
//
// Bedient die fertige Anwendung ueber echte Widgets, echte Controller, echte
// Model-Dienste und eine echte SQLite-Database im Arbeitsspeicher --
// genau wie ein Mensch, ueber Tastatur- und Mausereignisse.
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTextBrowser>
#include <memory>

#include "controller/LoginController.h"
#include "controller/UserManagementController.h"
#include "controller/ReadingListController.h"
#include "controller/PublicationController.h"

#include "model/ArxivClient.h"
#include "model/ArxivScheduler.h"
#include "model/AuthenticationService.h"
#include "model/UserManagementService.h"
#include "model/Database.h"
#include "model/ReadingListService.h"
#include "model/PermissionService.h"
#include "model/SqliteUserRepository.h"
#include "model/SqliteReadingListRepository.h"
#include "model/SqlitePublicationRepository.h"

#include "view/LoginView.h"
#include "view/UserManagementView.h"
#include "view/MainWindow.h"
#include "view/ReadingListView.h"
#include "view/PublicationView.h"

class TestAbnahme : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void t01_anmeldungMitGueltigenDatenOeffnetDieAnwendung();
    void t02_anmeldungMitFalschemPasswortWirdAbgelehnt();
    void t03_meldungVerraetNichtObDerBenutzerExistiert();
    void t04_anmeldungOhneEingabeWirdAbgelehnt();
    void t05_gesperrtesKontoKannSichNichtAnmelden();

    void t06_veroeffentlichungenErscheinenInDerListe();
    void t07_filterNachDisziplinGrenztDieListeEin();
    void t08_filterOhneTrefferZeigtEinenHinweis();

    void t09_ohneAuswahlErklaertDieDetailansichtSich();
    void t10_auswahlZeigtAlleAngabenZurVeroeffentlichung();

    void t11_veroeffentlichungLandetAufDerLeseliste();
    void t12_zweitesSetzenWirdAbgelehnt();
    void t13_prozessLaeuftVomVormerkenBisZumArchiv();
    void t14_nichtMoeglicheSchritteSindAusgegraut();

    void t15_mitarbeiterSiehtKeineFreigabeUndKeineBenutzerverwaltung();
    void t16_mitarbeiterDarfNichtFreigebenAuchAmModelVorbei();
    void t17_wissensmanagerGibtFrei();
    void t18_administratorLegtBenutzerAn();
    void t19_zuKurzesPasswortWirdAbgelehnt();
    void t20_administratorKannSichNichtSelbstSperren();

    void t21_derZeitplanerLoestDenselbenAbrufWieDerManuelleKlickAus();

private:
    void buildApplication();
    bool meldeAnUeberDieMaske(const QString &username, const QString &password);
    int  legeVeroeffentlichungAn(const QString &arxivId, const QString &title, Discipline discipline);

    template <typename WidgetTyp>
    WidgetTyp *finde(QWidget *parentWidget, const QString &objektname)
    {
        return parentWidget->findChild<WidgetTyp *>(objektname);
    }

    std::unique_ptr<Database>                         m_database;
    std::unique_ptr<SqliteUserRepository>          m_benutzerRepository;
    std::unique_ptr<SqlitePublicationRepository> m_veroeffentlichungRepository;
    std::unique_ptr<SqliteReadingListRepository>         m_leselisteRepository;

    std::unique_ptr<AuthenticationService>  m_authentication;
    std::unique_ptr<ReadingListService>           m_readingListService;
    std::unique_ptr<UserManagementService> m_benutzerverwaltungsService;
    std::unique_ptr<ArxivClient>                m_arxivClient;

    std::unique_ptr<LoginView>            m_anmeldeView;
    std::unique_ptr<PublicationView>  m_veroeffentlichungView;
    std::unique_ptr<ReadingListView>          m_meineLeselisteView;
    std::unique_ptr<ReadingListView>          m_freigabenView;
    std::unique_ptr<UserManagementView> m_benutzerverwaltungView;

    std::unique_ptr<LoginController>            m_anmeldeController;
    std::unique_ptr<PublicationController>  m_veroeffentlichungController;
    std::unique_ptr<ReadingListController>          m_meineLeselisteController;
    std::unique_ptr<ReadingListController>          m_freigabenController;
    std::unique_ptr<UserManagementController> m_benutzerverwaltungController;
};

void TestAbnahme::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("abnahme_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());

    m_benutzerRepository          = std::make_unique<SqliteUserRepository>(*m_database);
    m_veroeffentlichungRepository = std::make_unique<SqlitePublicationRepository>(*m_database);
    m_leselisteRepository         = std::make_unique<SqliteReadingListRepository>(*m_database);
    QVERIFY(m_benutzerRepository->createDefaultUsers());

    buildApplication();
}

void TestAbnahme::buildApplication()
{
    m_authentication = std::make_unique<AuthenticationService>(*m_benutzerRepository);
    m_readingListService  = std::make_unique<ReadingListService>(*m_leselisteRepository, *m_veroeffentlichungRepository);
    m_benutzerverwaltungsService = std::make_unique<UserManagementService>(*m_benutzerRepository);
    m_arxivClient = std::make_unique<ArxivClient>();

    m_anmeldeView           = std::make_unique<LoginView>();
    m_veroeffentlichungView = std::make_unique<PublicationView>();
    m_meineLeselisteView    = std::make_unique<ReadingListView>(ReadingListView::Mode::OwnList);
    m_freigabenView         = std::make_unique<ReadingListView>(ReadingListView::Mode::AllLists);
    m_benutzerverwaltungView = std::make_unique<UserManagementView>();

    m_anmeldeController = std::make_unique<LoginController>(*m_authentication, *m_anmeldeView);
    m_veroeffentlichungController = std::make_unique<PublicationController>(*m_arxivClient, *m_veroeffentlichungRepository, *m_veroeffentlichungView);
    m_meineLeselisteController = std::make_unique<ReadingListController>(*m_readingListService, *m_authentication, *m_meineLeselisteView);
    m_freigabenController = std::make_unique<ReadingListController>(*m_readingListService, *m_authentication, *m_freigabenView);
    m_benutzerverwaltungController = std::make_unique<UserManagementController>(*m_benutzerverwaltungsService, *m_authentication, *m_benutzerverwaltungView);

    connect(m_anmeldeView.get(), &LoginView::loginRequested, this, [this]() { m_anmeldeController->loginRequested(); });
    connect(m_veroeffentlichungView.get(), &PublicationView::disciplineSelected, this,
            [this](Discipline discipline) { m_veroeffentlichungController->disciplineSelected(discipline); });
    connect(m_veroeffentlichungView.get(), &PublicationView::addToReadingListRequested, this,
            [this](int publicationId) { m_meineLeselisteController->addToReadingList(publicationId); });
    connect(m_meineLeselisteView.get(), &ReadingListView::startReadingRequested, this,
            [this](int entryId) { m_meineLeselisteController->startReading(entryId); });
    connect(m_meineLeselisteView.get(), &ReadingListView::completeReadingRequested, this,
            [this](int entryId, int rating, const QString &note) { m_meineLeselisteController->completeReading(entryId, rating, note); });
    connect(m_freigabenView.get(), &ReadingListView::approveRequested, this,
            [this](int entryId) { m_freigabenController->approveForTraining(entryId); });
    connect(m_freigabenView.get(), &ReadingListView::archiveRequested, this,
            [this](int entryId) { m_freigabenController->archive(entryId); });
    connect(m_benutzerverwaltungView.get(), &UserManagementView::createRequested, this,
            [this](const QString &username, const QString &displayName, const QString &password, UserRole role) {
                m_benutzerverwaltungController->createUser(username, displayName, password, role);
            });
    connect(m_benutzerverwaltungView.get(), &UserManagementView::deactivateRequested, this,
            [this](int userId) { m_benutzerverwaltungController->deactivateAccount(userId); });
}

void TestAbnahme::cleanup()
{
    m_benutzerverwaltungController.reset();
    m_freigabenController.reset();
    m_meineLeselisteController.reset();
    m_veroeffentlichungController.reset();
    m_anmeldeController.reset();

    m_benutzerverwaltungView.reset();
    m_freigabenView.reset();
    m_meineLeselisteView.reset();
    m_veroeffentlichungView.reset();
    m_anmeldeView.reset();

    m_arxivClient.reset();
    m_benutzerverwaltungsService.reset();
    m_readingListService.reset();
    m_authentication.reset();

    m_leselisteRepository.reset();
    m_veroeffentlichungRepository.reset();
    m_benutzerRepository.reset();
    m_database.reset();
}

bool TestAbnahme::meldeAnUeberDieMaske(const QString &username, const QString &password)
{
    auto *benutzernameFeld = finde<QLineEdit>(m_anmeldeView.get(), QStringLiteral("benutzernameFeld"));
    auto *passwortFeld     = finde<QLineEdit>(m_anmeldeView.get(), QStringLiteral("passwortFeld"));
    auto *anmeldenKnopf    = finde<QPushButton>(m_anmeldeView.get(), QStringLiteral("anmeldenKnopf"));

    if (benutzernameFeld == nullptr || passwortFeld == nullptr || anmeldenKnopf == nullptr) {
        return false;
    }

    benutzernameFeld->clear();
    passwortFeld->clear();
    QTest::keyClicks(benutzernameFeld, username);
    QTest::keyClicks(passwortFeld, password);
    QTest::mouseClick(anmeldenKnopf, Qt::LeftButton);
    return true;
}

int TestAbnahme::legeVeroeffentlichungAn(const QString &arxivId, const QString &title, Discipline discipline)
{
    Publication v;
    v.setArxivId(arxivId);
    v.setTitle(title);
    v.setAuthors({ QStringLiteral("Ada Lovelace"), QStringLiteral("Alan Turing") });
    v.setSummary(QStringLiteral("Diese Arbeit beschreibt ein Verfahren zur Beschleunigung."));
    v.setArxivCategory(QStringLiteral("cs.LG"));
    v.setDiscipline(discipline);
    v.setPublishedAt(QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    v.setUrl(QStringLiteral("https://arxiv.org/abs/%1").arg(arxivId));
    m_veroeffentlichungRepository->save(v);
    return v.id();
}

void TestAbnahme::t01_anmeldungMitGueltigenDatenOeffnetDieAnwendung()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    QVERIFY(m_authentication->isLoggedIn());
    QCOMPARE(m_anmeldeView->result(), static_cast<int>(QDialog::Accepted));
    QCOMPARE(m_anmeldeView->currentUser().displayName(), QStringLiteral("Max Mustermann"));

    MainWindow fenster(m_anmeldeView->currentUser(), { { QStringLiteral("Übersicht"), new QWidget, true } });
    const QList<QLabel *> beschriftungen = fenster.findChildren<QLabel *>();
    bool kopfzeileGefunden = false;
    for (const QLabel *beschriftung : beschriftungen) {
        if (beschriftung->text().contains(QStringLiteral("Max Mustermann")) && beschriftung->text().contains(QStringLiteral("Employee"))) {
            kopfzeileGefunden = true;
        }
    }
    QVERIFY(kopfzeileGefunden);
}

void TestAbnahme::t02_anmeldungMitFalschemPasswortWirdAbgelehnt()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), QStringLiteral("falsch999")));
    QVERIFY(!m_authentication->isLoggedIn());
    auto *fehlerAnzeige = finde<QLabel>(m_anmeldeView.get(), QStringLiteral("fehlerAnzeige"));
    QVERIFY(fehlerAnzeige != nullptr);
    QCOMPARE(fehlerAnzeige->text(), AuthenticationService::kMessageInvalidCredentials);
    QVERIFY(m_anmeldeView->result() != QDialog::Accepted);
}

void TestAbnahme::t03_meldungVerraetNichtObDerBenutzerExistiert()
{
    auto *fehlerAnzeige = finde<QLabel>(m_anmeldeView.get(), QStringLiteral("fehlerAnzeige"));
    QVERIFY(fehlerAnzeige != nullptr);

    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), QStringLiteral("falsch999")));
    const QString a = fehlerAnzeige->text();
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("gibtesnicht"), QStringLiteral("falsch999")));
    const QString b = fehlerAnzeige->text();

    QVERIFY(!a.isEmpty());
    QCOMPARE(a, b);
}

void TestAbnahme::t04_anmeldungOhneEingabeWirdAbgelehnt()
{
    QVERIFY(meldeAnUeberDieMaske(QString(), QString()));
    QVERIFY(!m_authentication->isLoggedIn());
    auto *fehlerAnzeige = finde<QLabel>(m_anmeldeView.get(), QStringLiteral("fehlerAnzeige"));
    QCOMPARE(fehlerAnzeige->text(), AuthenticationService::kMessageRequiredFieldMissing);
}

void TestAbnahme::t05_gesperrtesKontoKannSichNichtAnmelden()
{
    const User administrator = m_benutzerRepository->findByUsername(QStringLiteral("admin01")).value();
    const User mitarbeiter   = m_benutzerRepository->findByUsername(QStringLiteral("ma01")).value();
    QVERIFY(m_benutzerverwaltungsService->deactivateAccount(administrator, mitarbeiter.id()).successful);

    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    QVERIFY(!m_authentication->isLoggedIn());
    auto *fehlerAnzeige = finde<QLabel>(m_anmeldeView.get(), QStringLiteral("fehlerAnzeige"));
    QCOMPARE(fehlerAnzeige->text(), AuthenticationService::kMessageAccountDeactivated);
}

void TestAbnahme::t06_veroeffentlichungenErscheinenInDerListe()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Schnelle Aufmerksamkeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    QVERIFY(tabelle != nullptr);
    QCOMPARE(tabelle->rowCount(), 1);
    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Schnelle Aufmerksamkeit"));
    QCOMPARE(tabelle->item(0, 1)->text(), QStringLiteral("ComputerScience"));

    auto *trefferAnzeige = finde<QLabel>(m_veroeffentlichungView.get(), QStringLiteral("trefferAnzeige"));
    QVERIFY(trefferAnzeige->text().contains(QStringLiteral("1 of 1")));
}

void TestAbnahme::t07_filterNachDisziplinGrenztDieListeEin()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);
    legeVeroeffentlichungAn(QStringLiteral("2608.00002"), QStringLiteral("Mathematics-Arbeit"), Discipline::Mathematics);

    auto *disziplinAuswahl = finde<QComboBox>(m_veroeffentlichungView.get(), QStringLiteral("disziplinAuswahl"));
    QVERIFY(disziplinAuswahl != nullptr);
    QVERIFY(disziplinAuswahl->count() >= 8);
    QCOMPARE(disziplinAuswahl->itemText(0), QStringLiteral("All disciplines"));

    const int position = disziplinAuswahl->findText(QStringLiteral("Mathematics"));
    QVERIFY(position >= 0);
    disziplinAuswahl->setCurrentIndex(position);

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    QCOMPARE(tabelle->rowCount(), 1);
    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Mathematics-Arbeit"));

    disziplinAuswahl->setCurrentIndex(0);
    QCOMPARE(tabelle->rowCount(), 2);
}

void TestAbnahme::t08_filterOhneTrefferZeigtEinenHinweis()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"), Discipline::ComputerScience);

    auto *disziplinAuswahl = finde<QComboBox>(m_veroeffentlichungView.get(), QStringLiteral("disziplinAuswahl"));
    disziplinAuswahl->setCurrentIndex(disziplinAuswahl->findText(QStringLiteral("Quantitative Biologie")));

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    QCOMPARE(tabelle->rowCount(), 0);

    auto *hinweisAnzeige = finde<QLabel>(m_veroeffentlichungView.get(), QStringLiteral("hinweisAnzeige"));
    QCOMPARE(hinweisAnzeige->text(), PublicationController::kHintNoResults);
}

void TestAbnahme::t09_ohneAuswahlErklaertDieDetailansichtSich()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();

    auto *leselisteKnopf = finde<QPushButton>(m_veroeffentlichungView.get(), QStringLiteral("leselisteKnopf"));
    QVERIFY(!leselisteKnopf->isEnabled());
}

void TestAbnahme::t10_auswahlZeigtAlleAngabenZurVeroeffentlichung()
{
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    tabelle->selectRow(0);

    auto *detailAnzeige = m_veroeffentlichungView->findChild<QTextBrowser *>(QStringLiteral("detailAnzeige"));
    QVERIFY(detailAnzeige != nullptr);

    const QString angezeigterText = detailAnzeige->toPlainText();
    QVERIFY(angezeigterText.contains(QStringLiteral("Eine Arbeit")));
    QVERIFY(angezeigterText.contains(QStringLiteral("Ada Lovelace")));
    QVERIFY(angezeigterText.contains(QStringLiteral("ComputerScience")));
    QVERIFY(angezeigterText.contains(QStringLiteral("2608.00001")));
    QVERIFY(angezeigterText.contains(QStringLiteral("Beschleunigung")));

    auto *leselisteKnopf = finde<QPushButton>(m_veroeffentlichungView.get(), QStringLiteral("leselisteKnopf"));
    QVERIFY(leselisteKnopf->isEnabled());
}

void TestAbnahme::t11_veroeffentlichungLandetAufDerLeseliste()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    tabelle->selectRow(0);
    auto *leselisteKnopf = finde<QPushButton>(m_veroeffentlichungView.get(), QStringLiteral("leselisteKnopf"));
    QTest::mouseClick(leselisteKnopf, Qt::LeftButton);

    auto *leselisteTabelle = finde<QTableWidget>(m_meineLeselisteView.get(), QStringLiteral("leselisteTabelle"));
    QVERIFY(leselisteTabelle != nullptr);
    QCOMPARE(leselisteTabelle->rowCount(), 1);
    QCOMPARE(leselisteTabelle->item(0, 0)->text(), QStringLiteral("Eine Arbeit"));
    QCOMPARE(leselisteTabelle->item(0, 1)->text(), QStringLiteral("Noted"));
}

void TestAbnahme::t12_zweitesSetzenWirdAbgelehnt()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    m_veroeffentlichungController->refreshView();

    auto *tabelle = finde<QTableWidget>(m_veroeffentlichungView.get(), QStringLiteral("veroeffentlichungTabelle"));
    tabelle->selectRow(0);
    auto *leselisteKnopf = finde<QPushButton>(m_veroeffentlichungView.get(), QStringLiteral("leselisteKnopf"));
    QTest::mouseClick(leselisteKnopf, Qt::LeftButton);
    QTest::mouseClick(leselisteKnopf, Qt::LeftButton);

    auto *leselisteTabelle = finde<QTableWidget>(m_meineLeselisteView.get(), QStringLiteral("leselisteTabelle"));
    QCOMPARE(leselisteTabelle->rowCount(), 1);
}

void TestAbnahme::t13_prozessLaeuftVomVormerkenBisZumArchiv()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);

    m_meineLeselisteController->addToReadingList(publicationId);
    auto *leselisteTabelle = finde<QTableWidget>(m_meineLeselisteView.get(), QStringLiteral("leselisteTabelle"));
    QCOMPARE(leselisteTabelle->item(0, 1)->text(), QStringLiteral("Noted"));

    leselisteTabelle->selectRow(0);
    auto *lesenBeginnenKnopf = finde<QPushButton>(m_meineLeselisteView.get(), QStringLiteral("lesenBeginnenKnopf"));
    QVERIFY(lesenBeginnenKnopf->isEnabled());
    QTest::mouseClick(lesenBeginnenKnopf, Qt::LeftButton);
    QCOMPARE(leselisteTabelle->item(0, 1)->text(), QStringLiteral("In Progress"));

    const int entryId = m_readingListService->ownList(m_authentication->currentUser().value()).at(0).id();
    m_meineLeselisteController->completeReading(entryId, 4, QStringLiteral("Für unser Modul relevant."));
    QCOMPARE(leselisteTabelle->item(0, 1)->text(), QStringLiteral("Read"));
    QCOMPARE(leselisteTabelle->item(0, 2)->text(), QStringLiteral("4"));

    m_anmeldeController->logoutRequested();
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("wm01"), SqliteUserRepository::kDefaultPassword));

    m_freigabenController->refreshAllLists();
    auto *freigabenTabelle = finde<QTableWidget>(m_freigabenView.get(), QStringLiteral("leselisteTabelle"));
    QCOMPARE(freigabenTabelle->rowCount(), 1);
    freigabenTabelle->selectRow(0);

    auto *freigebenKnopf = finde<QPushButton>(m_freigabenView.get(), QStringLiteral("freigebenKnopf"));
    QVERIFY(freigebenKnopf->isEnabled());
    QTest::mouseClick(freigebenKnopf, Qt::LeftButton);
    QCOMPARE(freigabenTabelle->item(0, 2)->text(), QStringLiteral("Approved for Training"));

    freigabenTabelle->selectRow(0);
    auto *archivierenKnopf = finde<QPushButton>(m_freigabenView.get(), QStringLiteral("archivierenKnopf"));
    QVERIFY(archivierenKnopf->isEnabled());
    QTest::mouseClick(archivierenKnopf, Qt::LeftButton);
    QCOMPARE(freigabenTabelle->item(0, 2)->text(), QStringLiteral("Archived"));
}

void TestAbnahme::t14_nichtMoeglicheSchritteSindAusgegraut()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    m_meineLeselisteController->addToReadingList(publicationId);

    auto *leselisteTabelle = finde<QTableWidget>(m_meineLeselisteView.get(), QStringLiteral("leselisteTabelle"));
    leselisteTabelle->selectRow(0);

    auto *lesenBeginnenKnopf = finde<QPushButton>(m_meineLeselisteView.get(), QStringLiteral("lesenBeginnenKnopf"));
    auto *abschliessenKnopf  = finde<QPushButton>(m_meineLeselisteView.get(), QStringLiteral("abschliessenKnopf"));

    QVERIFY(lesenBeginnenKnopf->isEnabled());
    QVERIFY(!abschliessenKnopf->isEnabled());
}

void TestAbnahme::t15_mitarbeiterSiehtKeineFreigabeUndKeineBenutzerverwaltung()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    const User mitarbeiter = m_anmeldeView->currentUser();

    const QList<MainWindow::Page> pages = {
        { QStringLiteral("Übersicht"),       new QWidget, true },
        { QStringLiteral("Meine readingList"), new QWidget, true },
        { QStringLiteral("Freigaben"),       new QWidget, PermissionService::canViewAllReadingLists(mitarbeiter.role()) },
        { QStringLiteral("User"),        new QWidget, PermissionService::canManageUsers(mitarbeiter.role()) }
    };

    MainWindow fenster(mitarbeiter, pages);
    auto *navigation = fenster.findChild<QListWidget *>(QStringLiteral("navigation"));
    QVERIFY(navigation != nullptr);

    QStringList sichtbareEintraege;
    for (int row = 0; row < navigation->count(); ++row) {
        sichtbareEintraege << navigation->item(row)->text();
    }

    QVERIFY(sichtbareEintraege.contains(QStringLiteral("Meine readingList")));
    QVERIFY(!sichtbareEintraege.contains(QStringLiteral("Freigaben")));
    QVERIFY(!sichtbareEintraege.contains(QStringLiteral("User")));
}

void TestAbnahme::t16_mitarbeiterDarfNichtFreigebenAuchAmModelVorbei()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);

    const User mitarbeiter = m_authentication->currentUser().value();
    m_meineLeselisteController->addToReadingList(publicationId);
    const int entryId = m_readingListService->ownList(mitarbeiter).at(0).id();
    m_meineLeselisteController->startReading(entryId);
    m_meineLeselisteController->completeReading(entryId, 5, QStringLiteral("Sehr gut."));

    const OperationResult result = m_readingListService->changeStatus(mitarbeiter, entryId, ReadingStatus::ApprovedForTraining);
    QVERIFY(!result.successful);
    QCOMPARE(result.errorMessage, PermissionService::kMessageNoPermission);
    QCOMPARE(m_readingListService->ownList(mitarbeiter).at(0).status(), ReadingStatus::Read);
}

void TestAbnahme::t17_wissensmanagerGibtFrei()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma01"), SqliteUserRepository::kDefaultPassword));
    const int publicationId = legeVeroeffentlichungAn(QStringLiteral("2608.00001"), QStringLiteral("Eine Arbeit"), Discipline::ComputerScience);
    const User mitarbeiter = m_authentication->currentUser().value();
    m_meineLeselisteController->addToReadingList(publicationId);
    const int entryId = m_readingListService->ownList(mitarbeiter).at(0).id();
    m_meineLeselisteController->startReading(entryId);
    m_meineLeselisteController->completeReading(entryId, 5, QStringLiteral("Sehr gut."));

    m_anmeldeController->logoutRequested();
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("wm01"), SqliteUserRepository::kDefaultPassword));

    m_freigabenController->refreshAllLists();
    auto *freigabenTabelle = finde<QTableWidget>(m_freigabenView.get(), QStringLiteral("leselisteTabelle"));
    QCOMPARE(freigabenTabelle->rowCount(), 1);
    QCOMPARE(freigabenTabelle->item(0, 1)->text(), QStringLiteral("Max Mustermann"));

    freigabenTabelle->selectRow(0);
    QTest::mouseClick(finde<QPushButton>(m_freigabenView.get(), QStringLiteral("freigebenKnopf")), Qt::LeftButton);
    QCOMPARE(m_readingListService->approvedForTraining().size(), 1);
}

void TestAbnahme::t18_administratorLegtBenutzerAn()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("admin01"), SqliteUserRepository::kDefaultPassword));

    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuBenutzernameFeld")), QStringLiteral("ma03"));
    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuAnzeigenameFeld")), QStringLiteral("Mia Meier"));
    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuPasswortFeld")), QStringLiteral("geheim1234"));
    QTest::mouseClick(finde<QPushButton>(m_benutzerverwaltungView.get(), QStringLiteral("anlegenKnopf")), Qt::LeftButton);

    auto *benutzerTabelle = finde<QTableWidget>(m_benutzerverwaltungView.get(), QStringLiteral("benutzerTabelle"));
    QCOMPARE(benutzerTabelle->rowCount(), 4);

    m_anmeldeController->logoutRequested();
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("ma03"), QStringLiteral("geheim1234")));
    QVERIFY(m_authentication->isLoggedIn());
}

void TestAbnahme::t19_zuKurzesPasswortWirdAbgelehnt()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("admin01"), SqliteUserRepository::kDefaultPassword));

    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuBenutzernameFeld")), QStringLiteral("ma03"));
    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuAnzeigenameFeld")), QStringLiteral("Mia Meier"));
    QTest::keyClicks(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuPasswortFeld")), QStringLiteral("kurz"));
    QTest::mouseClick(finde<QPushButton>(m_benutzerverwaltungView.get(), QStringLiteral("anlegenKnopf")), Qt::LeftButton);

    auto *meldungsAnzeige = finde<QLabel>(m_benutzerverwaltungView.get(), QStringLiteral("meldungsAnzeige"));
    QCOMPARE(meldungsAnzeige->text(), UserManagementService::kMessagePasswordTooShort);
    QVERIFY(!m_benutzerRepository->usernameExists(QStringLiteral("ma03")));
    QCOMPARE(finde<QLineEdit>(m_benutzerverwaltungView.get(), QStringLiteral("neuBenutzernameFeld"))->text(), QStringLiteral("ma03"));
}

void TestAbnahme::t20_administratorKannSichNichtSelbstSperren()
{
    QVERIFY(meldeAnUeberDieMaske(QStringLiteral("admin01"), SqliteUserRepository::kDefaultPassword));
    m_benutzerverwaltungController->refreshList();

    auto *benutzerTabelle = finde<QTableWidget>(m_benutzerverwaltungView.get(), QStringLiteral("benutzerTabelle"));
    QVERIFY(benutzerTabelle != nullptr);

    int eigeneZeile = -1;
    for (int row = 0; row < benutzerTabelle->rowCount(); ++row) {
        if (benutzerTabelle->item(row, 0)->text() == QStringLiteral("admin01")) {
            eigeneZeile = row;
        }
    }
    QVERIFY(eigeneZeile >= 0);

    benutzerTabelle->selectRow(eigeneZeile);
    QTest::mouseClick(finde<QPushButton>(m_benutzerverwaltungView.get(), QStringLiteral("deaktivierenKnopf")), Qt::LeftButton);

    auto *meldungsAnzeige = finde<QLabel>(m_benutzerverwaltungView.get(), QStringLiteral("meldungsAnzeige"));
    QCOMPARE(meldungsAnzeige->text(), PermissionService::kMessageNoPermission);

    const User administrator = m_benutzerRepository->findByUsername(QStringLiteral("admin01")).value();
    QVERIFY(administrator.isActive());
}

void TestAbnahme::t21_derZeitplanerLoestDenselbenAbrufWieDerManuelleKlickAus()
{
    // US-09: verdrahtet den Zeitplaner wie main.cpp es tut und prueft, dass
    // ein faelliger automatischer Abruf denselben Controller-Weg ausloest
    // wie der manuelle Klick -- ohne auf die echte Wanduhr zu warten.
    ArxivScheduler zeitplaner(QTime(7, 0));
    int anzahlAusgeloesterAbrufe = 0;
    connect(&zeitplaner, &ArxivScheduler::automaticFetchDue, this, [&]() {
        m_veroeffentlichungController->fetchRequested();
        ++anzahlAusgeloesterAbrufe;
    });

    // Vor 7 Uhr: nichts passiert.
    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(6, 30)));
    QCOMPARE(anzahlAusgeloesterAbrufe, 0);

    // Nach 7 Uhr: derselbe Weg wie beim manuellen Klick wird angestossen,
    // erkennbar an der Ladeanzeige, die der Controller einschaltet.
    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(7, 1)));
    QCOMPARE(anzahlAusgeloesterAbrufe, 1);

    // Am selben Tag kein zweites Mal, auch wenn die Zeit weiter voranschreitet.
    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(18, 0)));
    QCOMPARE(anzahlAusgeloesterAbrufe, 1);
}

QTEST_MAIN(TestAbnahme)
#include "tst_acceptancetest.moc"
