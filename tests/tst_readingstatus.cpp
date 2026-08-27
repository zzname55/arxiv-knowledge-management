// ---------------------------------------------------------------------------
// Test: ReadingStatus (Statusautomat, mandatory requirement 1.1)
// ---------------------------------------------------------------------------
#include <QtTest>
#include "model/ReadingStatus.h"

class TestLesestatus : public QObject
{
    Q_OBJECT

private slots:
    void lesestatusAlsText_liefertDeutscheBezeichnung();
    void umwandlung_istInBeideRichtungenVerlustfrei();
    void alleLesestatus_beschreibtDenVollstaendigenProzess();
    void istErlaubterUebergang_erlaubtDieSchritteDesProzesses();
    void istErlaubterUebergang_lehntStatussprungAb();
    void istErlaubterUebergang_lehntRueckwaertsschrittAb();
    void istErlaubterUebergang_lehntUebergangAufSichSelbstAb();
    void istErlaubterUebergang_lehntJedenUebergangAusDemEndzustandAb();
    void naechsterStatus_liefertDenFolgeschritt();
    void naechsterStatus_liefertNichtsImEndzustand();
    void istEndzustand_erkenntArchiviert();
    void erfordertBewertung_giltNurFuerDenAbschlussDesLesens();
};

void TestLesestatus::lesestatusAlsText_liefertDeutscheBezeichnung()
{
    QCOMPARE(readingStatusToText(ReadingStatus::Noted), QStringLiteral("Noted"));
    QCOMPARE(readingStatusToText(ReadingStatus::InProgress), QStringLiteral("In Progress"));
    QCOMPARE(readingStatusToText(ReadingStatus::Read), QStringLiteral("Read"));
    QCOMPARE(readingStatusToText(ReadingStatus::ApprovedForTraining), QStringLiteral("Approved for Training"));
    QCOMPARE(readingStatusToText(ReadingStatus::Archived), QStringLiteral("Archived"));
}

void TestLesestatus::umwandlung_istInBeideRichtungenVerlustfrei()
{
    for (const ReadingStatus status : allReadingStatuses()) {
        QCOMPARE(readingStatusFromText(readingStatusToText(status)).value(), status);
    }
    QVERIFY(!readingStatusFromText(QStringLiteral("Verschollen")).has_value());
}

void TestLesestatus::alleLesestatus_beschreibtDenVollstaendigenProzess()
{
    const QList<ReadingStatus> process = allReadingStatuses();
    QCOMPARE(process.size(), 5);
    QCOMPARE(process.first(), ReadingStatus::Noted);
    QCOMPARE(process.last(), ReadingStatus::Archived);
}

void TestLesestatus::istErlaubterUebergang_erlaubtDieSchritteDesProzesses()
{
    QVERIFY(isAllowedTransition(ReadingStatus::Noted, ReadingStatus::InProgress));
    QVERIFY(isAllowedTransition(ReadingStatus::InProgress, ReadingStatus::Read));
    QVERIFY(isAllowedTransition(ReadingStatus::Read, ReadingStatus::ApprovedForTraining));
    QVERIFY(isAllowedTransition(ReadingStatus::ApprovedForTraining, ReadingStatus::Archived));
}

void TestLesestatus::istErlaubterUebergang_lehntStatussprungAb()
{
    QVERIFY(!isAllowedTransition(ReadingStatus::Noted, ReadingStatus::Read));
    QVERIFY(!isAllowedTransition(ReadingStatus::Noted, ReadingStatus::ApprovedForTraining));
    QVERIFY(!isAllowedTransition(ReadingStatus::Noted, ReadingStatus::Archived));
    QVERIFY(!isAllowedTransition(ReadingStatus::InProgress, ReadingStatus::ApprovedForTraining));
    QVERIFY(!isAllowedTransition(ReadingStatus::Read, ReadingStatus::Archived));
}

void TestLesestatus::istErlaubterUebergang_lehntRueckwaertsschrittAb()
{
    QVERIFY(!isAllowedTransition(ReadingStatus::Read, ReadingStatus::InProgress));
    QVERIFY(!isAllowedTransition(ReadingStatus::InProgress, ReadingStatus::Noted));
    QVERIFY(!isAllowedTransition(ReadingStatus::Archived, ReadingStatus::Read));
}

void TestLesestatus::istErlaubterUebergang_lehntUebergangAufSichSelbstAb()
{
    for (const ReadingStatus status : allReadingStatuses()) {
        QVERIFY(!isAllowedTransition(status, status));
    }
}

void TestLesestatus::istErlaubterUebergang_lehntJedenUebergangAusDemEndzustandAb()
{
    for (const ReadingStatus ziel : allReadingStatuses()) {
        QVERIFY(!isAllowedTransition(ReadingStatus::Archived, ziel));
    }
}

void TestLesestatus::naechsterStatus_liefertDenFolgeschritt()
{
    QCOMPARE(nextStatus(ReadingStatus::Noted).value(), ReadingStatus::InProgress);
    QCOMPARE(nextStatus(ReadingStatus::InProgress).value(), ReadingStatus::Read);
    QCOMPARE(nextStatus(ReadingStatus::Read).value(), ReadingStatus::ApprovedForTraining);
    QCOMPARE(nextStatus(ReadingStatus::ApprovedForTraining).value(), ReadingStatus::Archived);
}

void TestLesestatus::naechsterStatus_liefertNichtsImEndzustand()
{
    QVERIFY(!nextStatus(ReadingStatus::Archived).has_value());
}

void TestLesestatus::istEndzustand_erkenntArchiviert()
{
    QVERIFY(isFinalState(ReadingStatus::Archived));
    QVERIFY(!isFinalState(ReadingStatus::Noted));
    QVERIFY(!isFinalState(ReadingStatus::Read));
}

void TestLesestatus::erfordertBewertung_giltNurFuerDenAbschlussDesLesens()
{
    QVERIFY(requiresRating(ReadingStatus::Read));
    QVERIFY(!requiresRating(ReadingStatus::Noted));
    QVERIFY(!requiresRating(ReadingStatus::InProgress));
    QVERIFY(!requiresRating(ReadingStatus::ApprovedForTraining));
    QVERIFY(!requiresRating(ReadingStatus::Archived));
}

QTEST_APPLESS_MAIN(TestLesestatus)
#include "tst_readingstatus.moc"
