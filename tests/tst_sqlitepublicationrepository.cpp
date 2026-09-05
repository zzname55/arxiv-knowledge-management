// ---------------------------------------------------------------------------
// Test: SqlitePublicationRepository
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QSqlError>
#include <QSqlQuery>
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

    void migrateOutdatedDisciplines_remapsLegacyLabelFromCategory();
    void migrateOutdatedDisciplines_leavesValidLabelsUntouched();
    void migrateOutdatedDisciplines_isIdempotent();
    void migrateOutdatedDisciplines_fallsBackToOtherForUnknownCategory();

private:
    std::unique_ptr<Database>                         m_database;
    std::unique_ptr<SqlitePublicationRepository> m_repository;

    static Publication neueVeroeffentlichung(const QString &arxivId, const QString &title,
                                                   Discipline discipline, const QDateTime &publishedAt);

    /// Writes a row carrying an arbitrary discipline text straight into the
    /// table. Needed because the repository only ever stores valid
    /// disciplines, while the migration case is precisely about legacy rows
    /// holding text that no longer resolves.
    void insertRowWithRawDisciplineText(const QString &arxivId, const QString &arxivCategory,
                                        const QString &disciplineText);

    /// Reads a row's raw discipline text back without going through the
    /// repository's conversion.
    QString rawDisciplineText(const QString &arxivId) const;
};

void TestSqliteVeroeffentlichungRepository::insertRowWithRawDisciplineText(
    const QString &arxivId, const QString &arxivCategory, const QString &disciplineText)
{
    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "INSERT INTO publication "
        "(arxiv_id, title, authors, summary, arxiv_category, discipline, published_at, url) "
        "VALUES (:arxiv_id, :title, :authors, :summary, :arxiv_category, :discipline, :published_at, :url)"));
    query.bindValue(QStringLiteral(":arxiv_id"),       arxivId);
    query.bindValue(QStringLiteral(":title"),          QStringLiteral("Legacy row %1").arg(arxivId));
    query.bindValue(QStringLiteral(":authors"),        QStringLiteral("A. Autorin"));
    query.bindValue(QStringLiteral(":summary"),        QStringLiteral("Kurzfassung."));
    query.bindValue(QStringLiteral(":arxiv_category"), arxivCategory);
    query.bindValue(QStringLiteral(":discipline"),     disciplineText);
    query.bindValue(QStringLiteral(":published_at"),
                    QDateTime(QDate(2026, 8, 26), QTime(8, 0), QTimeZone::UTC).toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":url"),            QStringLiteral("https://arxiv.org/abs/%1").arg(arxivId));
    QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
}

QString TestSqliteVeroeffentlichungRepository::rawDisciplineText(const QString &arxivId) const
{
    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral("SELECT discipline FROM publication WHERE arxiv_id = :arxiv_id"));
    query.bindValue(QStringLiteral(":arxiv_id"), arxivId);
    if (!query.exec() || !query.next()) {
        return QString();
    }
    return query.value(0).toString();
}

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
                                                     Discipline::GeneralPhysics, QDateTime(QDate(2026, 8, 13), QTime(8, 0), QTimeZone::UTC));
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

// ---------------------------------------------------------------------------
// Migration of legacy rows
//
// The "discipline" column holds the display label. When the catch-all
// discipline "Physics" was split into six real arXiv subfields, existing rows
// carrying the old text lost their assignment and fell back to "Other" on
// read. The raw category is still present in the "arxiv_category" column, so
// the assignment can be restored from it without any network request.
// ---------------------------------------------------------------------------

void TestSqliteVeroeffentlichungRepository::migrateOutdatedDisciplines_remapsLegacyLabelFromCategory()
{
    insertRowWithRawDisciplineText(QStringLiteral("2608.00001"), QStringLiteral("cond-mat.str-el"),
                                   QStringLiteral("Physik"));
    insertRowWithRawDisciplineText(QStringLiteral("2608.00002"), QStringLiteral("quant-ph"),
                                   QStringLiteral("Physik"));

    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 2);

    const auto firstRow = m_repository->findByArxivId(QStringLiteral("2608.00001"));
    QVERIFY(firstRow.has_value());
    QCOMPARE(firstRow->discipline(), Discipline::CondensedMatterPhysics);

    const auto secondRow = m_repository->findByArxivId(QStringLiteral("2608.00002"));
    QVERIFY(secondRow.has_value());
    QCOMPARE(secondRow->discipline(), Discipline::QuantumPhysicsAndGravitation);
}

void TestSqliteVeroeffentlichungRepository::migrateOutdatedDisciplines_leavesValidLabelsUntouched()
{
    Publication computerScience = neueVeroeffentlichung(QStringLiteral("2608.00001"), QStringLiteral("CS paper"),
                                                        Discipline::ComputerScience, QDateTime(QDate(2026, 8, 14), QTime(8, 0), QTimeZone::UTC));
    QVERIFY(m_repository->save(computerScience));

    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 0);
    QCOMPARE(rawDisciplineText(QStringLiteral("2608.00001")), disciplineToText(Discipline::ComputerScience));
}

void TestSqliteVeroeffentlichungRepository::migrateOutdatedDisciplines_isIdempotent()
{
    insertRowWithRawDisciplineText(QStringLiteral("2608.00001"), QStringLiteral("astro-ph.GA"),
                                   QStringLiteral("Physik"));

    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 1);
    // The second run finds nothing left to correct.
    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 0);
    QCOMPARE(rawDisciplineText(QStringLiteral("2608.00001")), disciplineToText(Discipline::Astrophysics));
}

void TestSqliteVeroeffentlichungRepository::migrateOutdatedDisciplines_fallsBackToOtherForUnknownCategory()
{
    // If the category cannot be mapped either, "Other" is all that remains.
    // What matters is that a valid label ends up in the column afterwards.
    insertRowWithRawDisciplineText(QStringLiteral("2608.00001"), QStringLiteral("xyz.ABC"),
                                   QStringLiteral("Completely unknown"));

    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 1);
    QCOMPARE(rawDisciplineText(QStringLiteral("2608.00001")), disciplineToText(Discipline::Other));
    QCOMPARE(m_repository->migrateOutdatedDisciplines(), 0);
}

QTEST_GUILESS_MAIN(TestSqliteVeroeffentlichungRepository)
#include "tst_sqlitepublicationrepository.moc"
