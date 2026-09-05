// ---------------------------------------------------------------------------
// Test: Discipline und Abfrageerzeugung fuer die arXiv-API
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QUrl>
#include <QUrlQuery>
#include "model/Discipline.h"

class TestDisziplin : public QObject
{
    Q_OBJECT

private slots:
    void disziplinAlsText_liefertDeutscheBezeichnung();
    void umwandlung_istInBeideRichtungenVerlustfrei();
    void alleDisziplinen_enthaeltDieGefordertenEintraege();
    void ausArxivKategorie_erkenntInformatik();
    void ausArxivKategorie_erkenntMathematik();
    void ausArxivKategorie_erkenntPhysikAuchBeiSonderkuerzeln();
    void ausArxivKategorie_erkenntStatistikBiologieWirtschaftElektrotechnik();
    void ausArxivKategorie_liefertSonstigeBeiUnbekanntemKuerzel();
    void ausArxivKategorie_kommtMitLeererEingabeZurecht();
    void abfrageUrl_zeigtAufDieOffizielleApi();
    void abfrageUrl_begrenztAufFuenfTreffer();
    void abfrageUrl_sortiertNachEinreichungsdatumAbsteigend();
    void abfrageUrl_beschraenktAufDieGewaehlteDisziplin();
    void abfrageUrl_umfasstBeiAlleDisziplinenMehrereKategorien();
};

void TestDisziplin::disziplinAlsText_liefertDeutscheBezeichnung()
{
    QCOMPARE(disciplineToText(Discipline::Alle), QStringLiteral("All disciplines"));
    QCOMPARE(disciplineToText(Discipline::ComputerScience), QStringLiteral("Computer Science"));
    QCOMPARE(disciplineToText(Discipline::Mathematics), QStringLiteral("Mathematics"));
    QCOMPARE(disciplineToText(Discipline::Astrophysics), QStringLiteral("Astrophysics"));
    QCOMPARE(disciplineToText(Discipline::CondensedMatterPhysics), QStringLiteral("Condensed Matter"));
    QCOMPARE(disciplineToText(Discipline::HighEnergyPhysics), QStringLiteral("High-Energy Physics"));
    QCOMPARE(disciplineToText(Discipline::QuantumPhysicsAndGravitation), QStringLiteral("Quantum Physics & Gravitation"));
    QCOMPARE(disciplineToText(Discipline::MathematicalPhysics), QStringLiteral("Mathematical Physics"));
    QCOMPARE(disciplineToText(Discipline::GeneralPhysics), QStringLiteral("General Physics"));
    QCOMPARE(disciplineToText(Discipline::Statistics), QStringLiteral("Statistics"));
    QCOMPARE(disciplineToText(Discipline::QuantitativeBiology), QStringLiteral("Quantitative Biology"));
    QCOMPARE(disciplineToText(Discipline::Economics), QStringLiteral("Economics"));
    QCOMPARE(disciplineToText(Discipline::ElectricalEngineering), QStringLiteral("Electrical Engineering"));
    QCOMPARE(disciplineToText(Discipline::Other), QStringLiteral("Other"));
}

void TestDisziplin::umwandlung_istInBeideRichtungenVerlustfrei()
{
    for (const Discipline discipline : allDisciplines()) {
        QCOMPARE(disciplineFromText(disciplineToText(discipline)).value(), discipline);
    }
    QVERIFY(!disciplineFromText(QStringLiteral("Alchemie")).has_value());
}

void TestDisziplin::alleDisziplinen_enthaeltDieGefordertenEintraege()
{
    const QList<Discipline> disziplinen = allDisciplines();
    QVERIFY(disziplinen.size() >= 13);
    QVERIFY(disziplinen.first() == Discipline::Alle);
}

void TestDisziplin::ausArxivKategorie_erkenntInformatik()
{
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("cs.LG")), Discipline::ComputerScience);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("cs.CL")), Discipline::ComputerScience);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("cs")), Discipline::ComputerScience);
}

void TestDisziplin::ausArxivKategorie_erkenntMathematik()
{
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("math.NT")), Discipline::Mathematics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("math.AG")), Discipline::Mathematics);
}

void TestDisziplin::ausArxivKategorie_erkenntPhysikAuchBeiSonderkuerzeln()
{
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("physics.optics")), Discipline::GeneralPhysics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("astro-ph.GA")), Discipline::Astrophysics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("cond-mat.str-el")), Discipline::CondensedMatterPhysics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("quant-ph")), Discipline::QuantumPhysicsAndGravitation);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("hep-th")), Discipline::HighEnergyPhysics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("gr-qc")), Discipline::QuantumPhysicsAndGravitation);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("nucl-ex")), Discipline::GeneralPhysics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("math-ph")), Discipline::MathematicalPhysics);
}

void TestDisziplin::ausArxivKategorie_erkenntStatistikBiologieWirtschaftElektrotechnik()
{
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("stat.ML")), Discipline::Statistics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("q-bio.NC")), Discipline::QuantitativeBiology);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("q-fin.PM")), Discipline::Economics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("econ.EM")), Discipline::Economics);
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("eess.SP")), Discipline::ElectricalEngineering);
}

void TestDisziplin::ausArxivKategorie_liefertSonstigeBeiUnbekanntemKuerzel()
{
    QCOMPARE(disciplineFromArxivCategory(QStringLiteral("xyz.ABC")), Discipline::Other);
}

void TestDisziplin::ausArxivKategorie_kommtMitLeererEingabeZurecht()
{
    QCOMPARE(disciplineFromArxivCategory(QString()), Discipline::Other);
}

void TestDisziplin::abfrageUrl_zeigtAufDieOffizielleApi()
{
    const QUrl url = buildArxivQueryUrl(Discipline::ComputerScience, 5);
    QCOMPARE(url.scheme(), QStringLiteral("https"));
    QCOMPARE(url.host(), QStringLiteral("export.arxiv.org"));
    QCOMPARE(url.path(), QStringLiteral("/api/query"));
}

void TestDisziplin::abfrageUrl_begrenztAufFuenfTreffer()
{
    const QUrlQuery query(buildArxivQueryUrl(Discipline::Alle, 15));
    QCOMPARE(query.queryItemValue(QStringLiteral("max_results")), QStringLiteral("15"));
}

void TestDisziplin::abfrageUrl_sortiertNachEinreichungsdatumAbsteigend()
{
    const QUrlQuery query(buildArxivQueryUrl(Discipline::Alle, 15));
    QCOMPARE(query.queryItemValue(QStringLiteral("sortBy")), QStringLiteral("submittedDate"));
    QCOMPARE(query.queryItemValue(QStringLiteral("sortOrder")), QStringLiteral("descending"));
}

void TestDisziplin::abfrageUrl_beschraenktAufDieGewaehlteDisziplin()
{
    const QUrlQuery query(buildArxivQueryUrl(Discipline::Mathematics, 5));
    const QString suchbegriff = query.queryItemValue(QStringLiteral("search_query"), QUrl::FullyDecoded);
    QVERIFY(suchbegriff.contains(QStringLiteral("cat:math")));
    QVERIFY(!suchbegriff.contains(QStringLiteral("cat:cs")));
}

void TestDisziplin::abfrageUrl_umfasstBeiAlleDisziplinenMehrereKategorien()
{
    const QUrlQuery query(buildArxivQueryUrl(Discipline::Alle, 15));
    const QString suchbegriff = query.queryItemValue(QStringLiteral("search_query"), QUrl::FullyDecoded);
    QVERIFY(suchbegriff.contains(QStringLiteral("cat:cs")));
    QVERIFY(suchbegriff.contains(QStringLiteral("cat:math")));
    QVERIFY(suchbegriff.contains(QStringLiteral("cat:physics")));
    QVERIFY(suchbegriff.contains(QStringLiteral("cat:quant-ph")));
    QVERIFY(!suchbegriff.contains(QStringLiteral("cat:quant-ph.*")));
    QVERIFY(suchbegriff.contains(QStringLiteral("OR")));
}

QTEST_APPLESS_MAIN(TestDisziplin)
#include "tst_discipline.moc"
