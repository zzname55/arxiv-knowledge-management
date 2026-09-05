// ---------------------------------------------------------------------------
// Test: ArxivAtomParser
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QDir>
#include <QFile>
#include "model/ArxivAtomParser.h"

class TestArxivAtomParser : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void parse_liestAlleEintraege();
    void parse_liestDieArxivIdOhneVersionUndPraefix();
    void parse_setztTitelInEineZeileZusammen();
    void parse_entferntUmbruecheUndLeerraumAusDerZusammenfassung();
    void parse_liestAlleAutoren();
    void parse_liestDasVeroeffentlichungsdatumAlsZeitpunkt();
    void parse_ordnetDieDisziplinAusDerHauptkategorieZu();
    void parse_baueDieArxivUrl();
    void parse_liefertLeereListeBeiFeedOhneEintraege();
    void parse_meldetFehlerBeiUngueltigemXml();
    void parse_meldetFehlerBeiLeererEingabe();
    void parse_uebergehtEintraegeOhneArxivId();

private:
    static QByteArray readSampleFile(const QString &fileName);
    QByteArray m_beispielAntwort;
};

QByteArray TestArxivAtomParser::readSampleFile(const QString &fileName)
{
    QFile file(QDir(QStringLiteral(TEST_DATA_DIR)).filePath(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Sample file %s could not be opened.", qPrintable(file.fileName()));
        return QByteArray();
    }
    return file.readAll();
}

void TestArxivAtomParser::initTestCase()
{
    m_beispielAntwort = readSampleFile(QStringLiteral("arxiv_response.xml"));
    QVERIFY(!m_beispielAntwort.isEmpty());
}

void TestArxivAtomParser::parse_liestAlleEintraege()
{
    QString failure;
    const QList<Publication> publications = ArxivAtomParser::parse(m_beispielAntwort, &failure);
    QVERIFY2(failure.isEmpty(), qPrintable(failure));
    QCOMPARE(publications.size(), 3);
}

void TestArxivAtomParser::parse_liestDieArxivIdOhneVersionUndPraefix()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    QCOMPARE(v.at(0).arxivId(), QStringLiteral("2608.01234"));
    QCOMPARE(v.at(1).arxivId(), QStringLiteral("2608.05678"));
}

void TestArxivAtomParser::parse_setztTitelInEineZeileZusammen()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    QCOMPARE(v.at(0).title(), QStringLiteral("Effiziente Aufmerksamkeitsmechanismen fuer lange Sequenzen"));
}

void TestArxivAtomParser::parse_entferntUmbruecheUndLeerraumAusDerZusammenfassung()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    const QString summary = v.at(0).summary();
    QVERIFY(!summary.startsWith(QLatin1Char(' ')));
    QVERIFY(!summary.contains(QLatin1Char('\n')));
    QCOMPARE(summary, QStringLiteral("Wir stellen ein Verfahren vor, das den Speicherbedarf von Aufmerksamkeitsmechanismen deutlich senkt."));
}

void TestArxivAtomParser::parse_liestAlleAutoren()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    QCOMPARE(v.at(0).authors().size(), 2);
    QCOMPARE(v.at(0).authors().at(0), QStringLiteral("Ashish Vaswani"));
    QCOMPARE(v.at(0).authors().at(1), QStringLiteral("Noam Shazeer"));
    QCOMPARE(v.at(1).authors().size(), 1);
}

void TestArxivAtomParser::parse_liestDasVeroeffentlichungsdatumAlsZeitpunkt()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    const QDateTime timestamp = v.at(0).publishedAt();
    QVERIFY(timestamp.isValid());
    QCOMPARE(timestamp.toUTC().date(), QDate(2026, 8, 14));
    QCOMPARE(timestamp.toUTC().time(), QTime(8, 15, 0));
}

void TestArxivAtomParser::parse_ordnetDieDisziplinAusDerHauptkategorieZu()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    QCOMPARE(v.at(0).arxivCategory(), QStringLiteral("cs.LG"));
    QCOMPARE(v.at(0).discipline(), Discipline::ComputerScience);
    QCOMPARE(v.at(1).discipline(), Discipline::Mathematics);
    QCOMPARE(v.at(2).discipline(), Discipline::QuantumPhysicsAndGravitation);
}

void TestArxivAtomParser::parse_baueDieArxivUrl()
{
    const QList<Publication> v = ArxivAtomParser::parse(m_beispielAntwort);
    QCOMPARE(v.at(0).url(), QStringLiteral("https://arxiv.org/abs/2608.01234"));
}

void TestArxivAtomParser::parse_liefertLeereListeBeiFeedOhneEintraege()
{
    const QByteArray leer = readSampleFile(QStringLiteral("arxiv_response_empty.xml"));
    QVERIFY(!leer.isEmpty());
    QString failure;
    const QList<Publication> v = ArxivAtomParser::parse(leer, &failure);
    QVERIFY(v.isEmpty());
    QVERIFY(failure.isEmpty());
}

void TestArxivAtomParser::parse_meldetFehlerBeiUngueltigemXml()
{
    QString failure;
    const QList<Publication> v = ArxivAtomParser::parse(QByteArrayLiteral("<feed><entry>abgeschnitten"), &failure);
    QVERIFY(v.isEmpty());
    QVERIFY(!failure.isEmpty());
}

void TestArxivAtomParser::parse_meldetFehlerBeiLeererEingabe()
{
    QString failure;
    const QList<Publication> v = ArxivAtomParser::parse(QByteArray(), &failure);
    QVERIFY(v.isEmpty());
    QVERIFY(!failure.isEmpty());
}

void TestArxivAtomParser::parse_uebergehtEintraegeOhneArxivId()
{
    const QByteArray antwort = readSampleFile(QStringLiteral("arxiv_response_missing_id.xml"));
    QVERIFY(!antwort.isEmpty());
    QString failure;
    const QList<Publication> v = ArxivAtomParser::parse(antwort, &failure);
    QVERIFY2(failure.isEmpty(), qPrintable(failure));
    QCOMPARE(v.size(), 1);
    QCOMPARE(v.at(0).title(), QStringLiteral("Vollstaendiger Eintrag"));
}

QTEST_APPLESS_MAIN(TestArxivAtomParser)
#include "tst_arxivatomparser.moc"
