// ---------------------------------------------------------------------------
// Test: Sortierung per Spaltenkopf (B-27)
// Prueft fachlich korrekte Sortierung UND dass die selection nach dem
// Sortieren noch auf den richtigen Datensatz zeigt (ueber die ID, nicht
// die Zeilennummer).
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTextBrowser>
#include <memory>

#include "view/UserManagementView.h"
#include "view/ReadingListView.h"
#include "view/PublicationView.h"

class TestSortierung : public QObject
{
    Q_OBJECT

private slots:
    void publications_sortierungIstEingeschaltet();
    void publications_spaltenkoepfeSindAnklickbar();
    void publications_nachTitelSortieren();
    void publications_zweiterKlickKehrtDieReihenfolgeUm();
    void publications_datumWirdChronologischSortiert();
    void publications_auswahlZeigtNachDemSortierenDieRichtigeArbeit();
    void publications_leselisteKnopfMeldetDieRichtigeArbeit();

    void readingList_sortierungIstEingeschaltet();
    void readingList_bewertungWirdNumerischSortiert();
    void readingList_eintraegeOhneBewertungStehenAufsteigendVorne();
    void readingList_aktionBetrifftNachDemSortierenDenRichtigenEintrag();

    void userManagement_sortierungIstEingeschaltet();
    void userManagement_auswahlBleibtNachDemSortierenRichtig();

private:
    static Publication baueVeroeffentlichung(int id, const QString &title, Discipline discipline, const QDate &date);
    static ReadingListEntry  baueEintrag(int id, const QString &title, ReadingStatus status, std::optional<int> rating);
    static User          baueBenutzer(int id, const QString &username, UserRole role);
    static int zeileMitText(QTableWidget *tabelle, const QString &text);
};

Publication TestSortierung::baueVeroeffentlichung(int id, const QString &title, Discipline discipline, const QDate &date)
{
    Publication v;
    v.setzeId(id);
    v.setzeArxivId(QStringLiteral("2608.%1").arg(id, 5, 10, QLatin1Char('0')));
    v.setzeTitel(title);
    v.setzeAutoren({ QStringLiteral("A. Autorin") });
    v.setzeZusammenfassung(QStringLiteral("Kurzfassung zu %1.").arg(title));
    v.setzeArxivKategorie(QStringLiteral("cs.LG"));
    v.setzeDisziplin(discipline);
    v.setzeVeroeffentlichtAm(QDateTime(date, QTime(8, 0), QTimeZone::UTC));
    v.setzeUrl(QStringLiteral("https://arxiv.org/abs/x"));
    return v;
}

ReadingListEntry TestSortierung::baueEintrag(int id, const QString &title, ReadingStatus status, std::optional<int> rating)
{
    ReadingListEntry entry;
    entry.setzeId(id);
    entry.setzeBenutzerId(1);
    entry.setzeVeroeffentlichungId(id);
    entry.setzeStatus(status);
    entry.setzeBewertung(rating);
    entry.setzeNotiz(QStringLiteral("Note zu %1").arg(title));
    entry.setzeErstelltAm(QDateTime(QDate(2026, 8, 10), QTime(8, 0), QTimeZone::UTC));
    entry.setzeGeaendertAm(entry.createdAt());
    entry.setzeVeroeffentlichungTitel(title);
    entry.setzeBenutzerAnzeigename(QStringLiteral("Max Mustermann"));
    return entry;
}

User TestSortierung::baueBenutzer(int id, const QString &username, UserRole role)
{
    User user;
    user.setzeId(id);
    user.setzeBenutzername(username);
    user.setzeAnzeigename(QStringLiteral("Anzeige %1").arg(username));
    user.setzeRolle(role);
    user.setActive(true);
    return user;
}

int TestSortierung::zeileMitText(QTableWidget *tabelle, const QString &text)
{
    for (int row = 0; row < tabelle->rowCount(); ++row) {
        if (tabelle->item(row, 0) != nullptr && tabelle->item(row, 0)->text() == text) {
            return row;
        }
    }
    return -1;
}

void TestSortierung::publications_sortierungIstEingeschaltet()
{
    PublicationView ansicht;
    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    QVERIFY(tabelle != nullptr);
    QVERIFY(tabelle->isSortingEnabled());
}

void TestSortierung::publications_spaltenkoepfeSindAnklickbar()
{
    PublicationView ansicht;
    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    QVERIFY(tabelle->horizontalHeader()->sectionsClickable());
    QVERIFY(tabelle->horizontalHeader()->isSortIndicatorShown());
}

void TestSortierung::publications_nachTitelSortieren()
{
    PublicationView ansicht;
    ansicht.showPublications({
        baueVeroeffentlichung(1, QStringLiteral("Zebra-Verfahren"), Discipline::ComputerScience, QDate(2026, 8, 14)),
        baueVeroeffentlichung(2, QStringLiteral("Ameisen-Algorithmus"), Discipline::Mathematics, QDate(2026, 8, 13)),
        baueVeroeffentlichung(3, QStringLiteral("Mittlere Arbeit"), Discipline::Physics, QDate(2026, 8, 12))
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);

    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Ameisen-Algorithmus"));
    QCOMPARE(tabelle->item(1, 0)->text(), QStringLiteral("Mittlere Arbeit"));
    QCOMPARE(tabelle->item(2, 0)->text(), QStringLiteral("Zebra-Verfahren"));
}

void TestSortierung::publications_zweiterKlickKehrtDieReihenfolgeUm()
{
    PublicationView ansicht;
    ansicht.showPublications({
        baueVeroeffentlichung(1, QStringLiteral("Zebra-Verfahren"), Discipline::ComputerScience, QDate(2026, 8, 14)),
        baueVeroeffentlichung(2, QStringLiteral("Ameisen-Algorithmus"), Discipline::Mathematics, QDate(2026, 8, 13))
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);
    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Ameisen-Algorithmus"));
    tabelle->sortByColumn(0, Qt::DescendingOrder);
    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Zebra-Verfahren"));
}

void TestSortierung::publications_datumWirdChronologischSortiert()
{
    PublicationView ansicht;
    ansicht.showPublications({
        baueVeroeffentlichung(1, QStringLiteral("Neunter"), Discipline::ComputerScience, QDate(2026, 8, 9)),
        baueVeroeffentlichung(2, QStringLiteral("Zehnter"), Discipline::ComputerScience, QDate(2026, 8, 10)),
        baueVeroeffentlichung(3, QStringLiteral("Erster im September"), Discipline::ComputerScience, QDate(2026, 9, 1))
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    tabelle->sortByColumn(2, Qt::AscendingOrder);

    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Neunter"));
    QCOMPARE(tabelle->item(1, 0)->text(), QStringLiteral("Zehnter"));
    QCOMPARE(tabelle->item(2, 0)->text(), QStringLiteral("Erster im September"));
}

void TestSortierung::publications_auswahlZeigtNachDemSortierenDieRichtigeArbeit()
{
    PublicationView ansicht;
    ansicht.showPublications({
        baueVeroeffentlichung(1, QStringLiteral("Zebra-Verfahren"), Discipline::ComputerScience, QDate(2026, 8, 14)),
        baueVeroeffentlichung(2, QStringLiteral("Ameisen-Algorithmus"), Discipline::Mathematics, QDate(2026, 8, 13))
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);
    tabelle->selectRow(0);

    auto *detailAnzeige = ansicht.findChild<QTextBrowser *>(QStringLiteral("detailAnzeige"));
    QVERIFY(detailAnzeige != nullptr);
    const QString angezeigterText = detailAnzeige->toPlainText();
    QVERIFY(angezeigterText.contains(QStringLiteral("Ameisen-Algorithmus")));
    QVERIFY(!angezeigterText.contains(QStringLiteral("Zebra-Verfahren")));
}

void TestSortierung::publications_leselisteKnopfMeldetDieRichtigeArbeit()
{
    PublicationView ansicht;
    ansicht.showPublications({
        baueVeroeffentlichung(11, QStringLiteral("Zebra-Verfahren"), Discipline::ComputerScience, QDate(2026, 8, 14)),
        baueVeroeffentlichung(22, QStringLiteral("Ameisen-Algorithmus"), Discipline::Mathematics, QDate(2026, 8, 13))
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("veroeffentlichungTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);
    tabelle->selectRow(0);

    QSignalSpy beobachter(&ansicht, &PublicationView::addToReadingListRequested);
    auto *leselisteKnopf = ansicht.findChild<QPushButton *>(QStringLiteral("leselisteKnopf"));
    QTest::mouseClick(leselisteKnopf, Qt::LeftButton);

    QCOMPARE(beobachter.count(), 1);
    QCOMPARE(beobachter.at(0).at(0).toInt(), 22);
}

void TestSortierung::readingList_sortierungIstEingeschaltet()
{
    ReadingListView ansicht(ReadingListView::Mode::OwnList);
    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("leselisteTabelle"));
    QVERIFY(tabelle != nullptr);
    QVERIFY(tabelle->isSortingEnabled());
}

void TestSortierung::readingList_bewertungWirdNumerischSortiert()
{
    ReadingListView ansicht(ReadingListView::Mode::OwnList);
    ansicht.showEntries({
        baueEintrag(1, QStringLiteral("Arbeit A"), ReadingStatus::Read, 5),
        baueEintrag(2, QStringLiteral("Arbeit B"), ReadingStatus::Read, 2),
        baueEintrag(3, QStringLiteral("Arbeit C"), ReadingStatus::Read, 4)
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("leselisteTabelle"));
    tabelle->sortByColumn(2, Qt::AscendingOrder);

    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Arbeit B"));
    QCOMPARE(tabelle->item(1, 0)->text(), QStringLiteral("Arbeit C"));
    QCOMPARE(tabelle->item(2, 0)->text(), QStringLiteral("Arbeit A"));
}

void TestSortierung::readingList_eintraegeOhneBewertungStehenAufsteigendVorne()
{
    ReadingListView ansicht(ReadingListView::Mode::OwnList);
    ansicht.showEntries({
        baueEintrag(1, QStringLiteral("Bewertet"), ReadingStatus::Read, 3),
        baueEintrag(2, QStringLiteral("Ohne Wertung"), ReadingStatus::Noted, std::nullopt)
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("leselisteTabelle"));
    tabelle->sortByColumn(2, Qt::AscendingOrder);

    QCOMPARE(tabelle->item(0, 0)->text(), QStringLiteral("Ohne Wertung"));
    QCOMPARE(tabelle->item(1, 0)->text(), QStringLiteral("Bewertet"));
}

void TestSortierung::readingList_aktionBetrifftNachDemSortierenDenRichtigenEintrag()
{
    ReadingListView ansicht(ReadingListView::Mode::OwnList);
    ansicht.showEntries({
        baueEintrag(11, QStringLiteral("Zebra"), ReadingStatus::Noted, std::nullopt),
        baueEintrag(22, QStringLiteral("Ameise"), ReadingStatus::Noted, std::nullopt)
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("leselisteTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);

    const int zeileAmeise = zeileMitText(tabelle, QStringLiteral("Ameise"));
    QCOMPARE(zeileAmeise, 0);
    tabelle->selectRow(zeileAmeise);

    QSignalSpy beobachter(&ansicht, &ReadingListView::startReadingRequested);
    auto *lesenBeginnenKnopf = ansicht.findChild<QPushButton *>(QStringLiteral("lesenBeginnenKnopf"));
    QVERIFY(lesenBeginnenKnopf->isEnabled());
    QTest::mouseClick(lesenBeginnenKnopf, Qt::LeftButton);

    QCOMPARE(beobachter.count(), 1);
    QCOMPARE(beobachter.at(0).at(0).toInt(), 22);
}

void TestSortierung::userManagement_sortierungIstEingeschaltet()
{
    UserManagementView ansicht;
    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("benutzerTabelle"));
    QVERIFY(tabelle != nullptr);
    QVERIFY(tabelle->isSortingEnabled());
}

void TestSortierung::userManagement_auswahlBleibtNachDemSortierenRichtig()
{
    UserManagementView ansicht;
    ansicht.showUsers({
        baueBenutzer(11, QStringLiteral("zz01"), UserRole::Employee),
        baueBenutzer(22, QStringLiteral("aa01"), UserRole::Administrator)
    });

    auto *tabelle = ansicht.findChild<QTableWidget *>(QStringLiteral("benutzerTabelle"));
    tabelle->sortByColumn(0, Qt::AscendingOrder);
    tabelle->selectRow(0);

    QSignalSpy beobachter(&ansicht, &UserManagementView::deactivateRequested);
    auto *deaktivierenKnopf = ansicht.findChild<QPushButton *>(QStringLiteral("deaktivierenKnopf"));
    QVERIFY(deaktivierenKnopf->isEnabled());
    QTest::mouseClick(deaktivierenKnopf, Qt::LeftButton);

    QCOMPARE(beobachter.count(), 1);
    QCOMPARE(beobachter.at(0).at(0).toInt(), 22);
}

QTEST_MAIN(TestSortierung)
#include "tst_sorting.moc"
