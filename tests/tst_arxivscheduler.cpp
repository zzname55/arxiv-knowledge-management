// ---------------------------------------------------------------------------
// Test: ArxivScheduler (US-09, automatischer taeglicher Abruf um 7 Uhr)
//
// Getestet wird ausschliesslich die deterministische Zeitlogik (istFaelig
// und checkNow mit einem erfundenen timestamp). start() haengt von der
// echten Wanduhr ab und wird bewusst nicht unit-getestet -- das kann nur
// durch tatsaechliches Laufenlassen der Anwendung geprueft werden.
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QSignalSpy>
#include "model/ArxivScheduler.h"

class TestArxivZeitplaner : public QObject
{
    Q_OBJECT

private slots:
    void istFaelig_vorDerAbrufzeitNichtFaellig();
    void istFaelig_nachDerAbrufzeitOhneVorherigenAbrufFaellig();
    void istFaelig_genauZurAbrufzeitFaellig();
    void istFaelig_amSelbenTagNachBereitsErfolgtemAbrufNichtFaellig();
    void istFaelig_amFolgetagNachErneutemUeberschreitenWiederFaellig();

    void pruefeJetzt_vorAbrufzeit_loestNichtAusUndSendetKeinSignal();
    void pruefeJetzt_nachAbrufzeitErstesMalHeute_loestAusUndSendetSignal();
    void pruefeJetzt_zweitesMalAmSelbenTag_loestNichtErneutAus();
    void pruefeJetzt_amFolgetagNachErneutemUeberschreiten_loestErneutAus();
    void pruefeJetzt_merktSichDenTagDesAusgeloestenAbrufs();

    void konstruktor_erlaubtEineAndereAbrufzeitAlsDenStandard();
};

void TestArxivZeitplaner::istFaelig_vorDerAbrufzeitNichtFaellig()
{
    const QDateTime now(QDate(2026, 8, 21), QTime(6, 59));
    QVERIFY(!ArxivScheduler::istFaelig(now, QTime(7, 0), std::nullopt));
}

void TestArxivZeitplaner::istFaelig_nachDerAbrufzeitOhneVorherigenAbrufFaellig()
{
    const QDateTime now(QDate(2026, 8, 21), QTime(9, 15));
    QVERIFY(ArxivScheduler::istFaelig(now, QTime(7, 0), std::nullopt));
}

void TestArxivZeitplaner::istFaelig_genauZurAbrufzeitFaellig()
{
    const QDateTime now(QDate(2026, 8, 21), QTime(7, 0, 0));
    QVERIFY(ArxivScheduler::istFaelig(now, QTime(7, 0), std::nullopt));
}

void TestArxivZeitplaner::istFaelig_amSelbenTagNachBereitsErfolgtemAbrufNichtFaellig()
{
    const QDate today(2026, 8, 21);
    const QDateTime now(today, QTime(10, 0));
    QVERIFY(!ArxivScheduler::istFaelig(now, QTime(7, 0), today));
}

void TestArxivZeitplaner::istFaelig_amFolgetagNachErneutemUeberschreitenWiederFaellig()
{
    const QDate gestern(2026, 8, 21);
    const QDateTime heuteFrueh(QDate(2026, 8, 22), QTime(7, 5));
    QVERIFY(ArxivScheduler::istFaelig(heuteFrueh, QTime(7, 0), gestern));
}

void TestArxivZeitplaner::pruefeJetzt_vorAbrufzeit_loestNichtAusUndSendetKeinSignal()
{
    ArxivScheduler zeitplaner(QTime(7, 0));
    QSignalSpy spy(&zeitplaner, &ArxivScheduler::automaticFetchDue);

    const bool triggered = zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(6, 30)));

    QVERIFY(!triggered);
    QCOMPARE(spy.count(), 0);
    QVERIFY(!zeitplaner.lastAutomaticFetchDay().has_value());
}

void TestArxivZeitplaner::pruefeJetzt_nachAbrufzeitErstesMalHeute_loestAusUndSendetSignal()
{
    ArxivScheduler zeitplaner(QTime(7, 0));
    QSignalSpy spy(&zeitplaner, &ArxivScheduler::automaticFetchDue);

    const bool triggered = zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(7, 1)));

    QVERIFY(triggered);
    QCOMPARE(spy.count(), 1);
}

void TestArxivZeitplaner::pruefeJetzt_zweitesMalAmSelbenTag_loestNichtErneutAus()
{
    // AK-09.2: pro Kalendertag hoechstens einmal.
    ArxivScheduler zeitplaner(QTime(7, 0));
    QSignalSpy spy(&zeitplaner, &ArxivScheduler::automaticFetchDue);

    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(7, 1)));
    const bool zweitesMal = zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(12, 0)));

    QVERIFY(!zweitesMal);
    QCOMPARE(spy.count(), 1);
}

void TestArxivZeitplaner::pruefeJetzt_amFolgetagNachErneutemUeberschreiten_loestErneutAus()
{
    ArxivScheduler zeitplaner(QTime(7, 0));
    QSignalSpy spy(&zeitplaner, &ArxivScheduler::automaticFetchDue);

    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(7, 1)));
    const bool folgetag = zeitplaner.checkNow(QDateTime(QDate(2026, 8, 22), QTime(7, 1)));

    QVERIFY(folgetag);
    QCOMPARE(spy.count(), 2);
}

void TestArxivZeitplaner::pruefeJetzt_merktSichDenTagDesAusgeloestenAbrufs()
{
    ArxivScheduler zeitplaner(QTime(7, 0));
    zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(7, 1)));
    QCOMPARE(zeitplaner.lastAutomaticFetchDay().value(), QDate(2026, 8, 21));
}

void TestArxivZeitplaner::konstruktor_erlaubtEineAndereAbrufzeitAlsDenStandard()
{
    // Nicht Teil des Auftrags, aber die Klasse soll nicht auf 7 Uhr
    // hartkodiert sein -- das haette einen kuenftigen Wunsch nach einer
    // anderen Uhrzeit unnoetig teuer gemacht.
    ArxivScheduler zeitplaner(QTime(12, 0));
    QVERIFY(!zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(11, 59))));
    QVERIFY(zeitplaner.checkNow(QDateTime(QDate(2026, 8, 21), QTime(12, 0))));
}

QTEST_APPLESS_MAIN(TestArxivZeitplaner)
#include "tst_arxivscheduler.moc"
