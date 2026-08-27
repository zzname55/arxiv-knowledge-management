// ---------------------------------------------------------------------------
// Test: SqlitePublicationRepository
// ---------------------------------------------------------------------------
#include <QtTest>
#include <memory>
#include "model/Database.h"
#include "model/SqlitePublicationRepository.h"

class TestSqliteVeroeffentlichungRepository : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void speichere_vergibtEineId();
    void speichere_erkenntBekannteArxivIdUndLegtKeinDuplikatAn();
    void speichere_aktualisiertBeiBekannterArxivIdDieAngaben();
    void findeAlle_liefertNeuesteZuerst();
    void findeAlle_begrenztAufDieGewuenschteAnzahl();
    void findeNachDisziplin_liefertNurDieGewaehlteDisziplin();
    void findeNachDisziplin_liefertBeiAlleSaemtlicheEintraege();
    void findeNachDisziplin_liefertLeereListeWennNichtsPasst();
    void findeNachId_liefertAlleFelderZurueck();
    void findeNachArxivId_findetDenEintrag();
    void autoren_ueberstehenDenWegDurchDieDatenbank();
    void count_zaehltAlleEintraege();

private:
    std::unique_ptr<Database>                         m_database;
    std::unique_ptr<SqlitePublicationRepository> m_repository;

    static Publication neueVeroeffentlichung(const QString &arxivId, const QString &title,
                                                   Discipline discipline, const QDateTime &publishedAt);
};

Publication TestSqliteVeroeffentlichungRepository::neueVeroeffentlichung(
    const QString &arxivId, const QString &title, Discipline discipline, const QDateTime &publishedAt)
{
    Publication v;
    v.setArxivId(arxivId);
    v.setTitle(title);
    v.setAuthors({ QStringLiteral("A. Autorin"), QStringLiteral("B. Autor") });
    v.setSummary(QStringLiteral("Kurzfassung zu %1.").arg(title));
    v.setArxivCategory(QStringLiteral("cs.LG"));
    v.setDiscipline(discipline);
    v.setPublishedAt(publishedAt);
    v.setUrl(QStringLiteral("https://arxiv.org/abs/%1").arg(arxivId));
    return v;
}

void TestSqliteVeroeffentlichungRepository::init()
{
    m_database = std::make_unique<Database>(QStringLiteral("test_pub_%1").arg(QString::fromLatin1(QTest::currentTestFunction())));
    QVERIFY(m_database->open(Database::kInMemoryPath));
    QVERIFY(m_database->createSchema());
    m_repository = std::make_unique<SqlitePublicationRepository>(*m_database);
}

void TestSqliteVeroeffentlichungRepository::cleanup()
{
    m_repository.reset();
    m_database.reset();
}

void TestSqliteVeroeffentlichungRepository::speichere_vergibtEineId()
{
    Publication v = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY2(m_repository->save(v), qPrintable(m_repository->lastError()));
    QVERIFY(v.isPersisted());
}

void TestSqliteVeroeffentlichungRepository::speichere_erkenntBekannteArxivIdUndLegtKeinDuplikatAn()
{
    Publication erste = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                    Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(erste));

    Publication erneut = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                     Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(erneut));

    QCOMPARE(m_repository->count(), 1);
    QCOMPARE(erneut.id(), erste.id());
}

void TestSqliteVeroeffentlichungRepository::speichere_aktualisiertBeiBekannterArxivIdDieAngaben()
{
    Publication erste = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Alter Title"),
                                                    Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(erste));

    Publication ueberarbeitet = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Neuer Title"),
                                                            Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(ueberarbeitet));

    const auto gelesen = m_repository->findByArxivId(QStringLiteral("2608.00001"));
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->title(), QStringLiteral("Neuer Title"));
}

void TestSqliteVeroeffentlichungRepository::findeAlle_liefertNeuesteZuerst()
{
    Publication aeltere = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Aeltere Arbeit"),
                                                      Discipline::ComputerScience, QDateTime(QDate(2026, 8, 10), QTime(8, 0), QTimeZone::UTC));
    Publication neuere = neueVeroeffentlichung(QStringLiteral("2608.00002"), QStringLiteral("Neuere Arbeit"),
                                                     Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(aeltere));
    QVERIFY(m_repository->save(neuere));

    const QList<Publication> found = m_repository->findByDiscipline(Discipline::Alle);
    QCOMPARE(found.size(), 2);
    QCOMPARE(found.at(0).title(), QStringLiteral("Neuere Arbeit"));
    QCOMPARE(found.at(1).title(), QStringLiteral("Aeltere Arbeit"));
}

void TestSqliteVeroeffentlichungRepository::findeAlle_begrenztAufDieGewuenschteAnzahl()
{
    for (int i = 1; i <= 8; ++i) {
        Publication v = neueVeroeffentlichung(QStringLiteral("2608.0000%1").arg(i), QStringLiteral("Arbeit %1").arg(i),
                                                    Discipline::ComputerScience, QDateTime(QDate(2026, 8, i), QTime(8, 0), QTimeZone::UTC));
        QVERIFY(m_repository->save(v));
    }

    const QList<Publication> found = m_repository->findByDiscipline(Discipline::Alle, 5);
    QCOMPARE(found.size(), 5);
    QCOMPARE(found.at(0).title(), QStringLiteral("Arbeit 8"));
}

void TestSqliteVeroeffentlichungRepository::findeNachDisziplin_liefertNurDieGewaehlteDisziplin()
{
    Publication informatik = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"),
                                                         Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    Publication mathematik = neueVeroeffentlichung(QStringLiteral("2608.00002"), QStringLiteral("Mathematics-Arbeit"),
                                                         Discipline::Mathematics, QDateTime(QDate(2026, 8, 13), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(informatik));
    QVERIFY(m_repository->save(mathematik));

    const QList<Publication> found = m_repository->findByDiscipline(Discipline::Mathematics);
    QCOMPARE(found.size(), 1);
    QCOMPARE(found.at(0).title(), QStringLiteral("Mathematics-Arbeit"));
}

void TestSqliteVeroeffentlichungRepository::findeNachDisziplin_liefertBeiAlleSaemtlicheEintraege()
{
    Publication informatik = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"),
                                                         Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    Publication physik = neueVeroeffentlichung(QStringLiteral("2608.00002"), QStringLiteral("Physics-Arbeit"),
                                                     Discipline::Physics, QDateTime(QDate(2026, 8, 13), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(informatik));
    QVERIFY(m_repository->save(physik));

    QCOMPARE(m_repository->findByDiscipline(Discipline::Alle).size(), 2);
}

void TestSqliteVeroeffentlichungRepository::findeNachDisziplin_liefertLeereListeWennNichtsPasst()
{
    Publication informatik = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("ComputerScience-Arbeit"),
                                                         Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(informatik));
    QVERIFY(m_repository->findByDiscipline(Discipline::QuantitativeBiology).isEmpty());
}

void TestSqliteVeroeffentlichungRepository::findeNachId_liefertAlleFelderZurueck()
{
    const QDateTime publishedAt(QDate(2026, 8, 14), QTime(8, 15), QTimeZone::UTC);
    Publication v = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"), Discipline::ComputerScience, publishedAt);
    QVERIFY(m_repository->save(v));

    const auto gelesen = m_repository->findById(v.id());
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->arxivId(), QStringLiteral("2608.00001"));
    QCOMPARE(gelesen->title(), QStringLiteral("Erste Arbeit"));
    QCOMPARE(gelesen->summary(), QStringLiteral("Kurzfassung zu Erste Arbeit."));
    QCOMPARE(gelesen->arxivCategory(), QStringLiteral("cs.LG"));
    QCOMPARE(gelesen->discipline(), Discipline::ComputerScience);
    QCOMPARE(gelesen->url(), QStringLiteral("https://arxiv.org/abs/2608.00001"));
    QCOMPARE(gelesen->publishedAt().toUTC(), publishedAt);
}

void TestSqliteVeroeffentlichungRepository::findeNachArxivId_findetDenEintrag()
{
    Publication v = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(v));
    QVERIFY(m_repository->findByArxivId(QStringLiteral("2608.00001")).has_value());
    QVERIFY(!m_repository->findByArxivId(QStringLiteral("9999.99999")).has_value());
}

void TestSqliteVeroeffentlichungRepository::autoren_ueberstehenDenWegDurchDieDatenbank()
{
    Publication v = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(v));

    const auto gelesen = m_repository->findById(v.id());
    QVERIFY(gelesen.has_value());
    QCOMPARE(gelesen->authors().size(), 2);
    QCOMPARE(gelesen->authors().at(0), QStringLiteral("A. Autorin"));
    QCOMPARE(gelesen->authors().at(1), QStringLiteral("B. Autor"));
}

void TestSqliteVeroeffentlichungRepository::count_zaehltAlleEintraege()
{
    QCOMPARE(m_repository->count(), 0);
    Publication v = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("Erste Arbeit"),
                                                Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(v));
    QCOMPARE(m_repository->count(), 1);
}

QTEST_GUILESS_MAIN(TestSqliteVeroeffentlichungRepository)
#include "tst_sqlitepublicationrepository.moc"
