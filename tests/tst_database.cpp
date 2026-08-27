// ---------------------------------------------------------------------------
// Test: Database (Schema und Verbindung)
// ---------------------------------------------------------------------------
#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "model/Database.h"

class TestDatenbank : public QObject
{
    Q_OBJECT

private slots:
    void oeffne_gelingtImArbeitsspeicher();
    void oeffne_scheitertBeiUngueltigemPfad();
    void erzeugeSchema_legtAlleTabellenAn();
    void erzeugeSchema_istMehrfachAufrufbar();
    void schema_benutzernameIstEindeutig();
    void schema_arxivIdIstEindeutig();
    void schema_verhindertDoppeltenLeselisteneintrag();
    void schema_loeschtEintraegeMitDerVeroeffentlichung();

private:
    QString eindeutigerVerbindungsname() const
    {
        return QStringLiteral("test_%1").arg(QString::fromLatin1(QTest::currentTestFunction()));
    }
};

void TestDatenbank::oeffne_gelingtImArbeitsspeicher()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY2(database.open(Database::kInMemoryPath), qPrintable(database.lastError()));
    QVERIFY(database.isOpen());
}

void TestDatenbank::oeffne_scheitertBeiUngueltigemPfad()
{
    Database database(eindeutigerVerbindungsname());
    const bool gelungen = database.open(QStringLiteral("Z:/gibt/es/nicht/wissen.db"));
    QVERIFY(!gelungen);
    QVERIFY(!database.lastError().isEmpty());
}

void TestDatenbank::erzeugeSchema_legtAlleTabellenAn()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY2(database.createSchema(), qPrintable(database.lastError()));

    const QStringList tabellen = database.connection().tables();
    QVERIFY(tabellen.contains(QStringLiteral("user")));
    QVERIFY(tabellen.contains(QStringLiteral("publication")));
    QVERIFY(tabellen.contains(QStringLiteral("reading_list_entry")));
}

void TestDatenbank::erzeugeSchema_istMehrfachAufrufbar()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY(database.createSchema());
    QVERIFY(database.createSchema());
}

void TestDatenbank::schema_benutzernameIstEindeutig()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY(database.createSchema());

    QSqlQuery query(database.connection());
    const QString insertStatement = QStringLiteral(
        "INSERT INTO user (username, displayName, password_hash, salt, role, is_active) "
        "VALUES ('ma01', 'Max', 'hash', 'salt', 'Employee', 1)");
    QVERIFY(query.exec(insertStatement));
    QVERIFY(!query.exec(insertStatement));
}

void TestDatenbank::schema_arxivIdIstEindeutig()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY(database.createSchema());

    QSqlQuery query(database.connection());
    const QString insertStatement = QStringLiteral(
        "INSERT INTO publication "
        "(arxiv_id, title, authors, summary, arxiv_category, discipline, published_at, url) "
        "VALUES ('2608.01234', 'Title', 'A. Autor', 'Kurzfassung', 'cs.LG', 'ComputerScience', '2026-08-14T09:00:00Z', 'https://arxiv.org/abs/2608.01234')");
    QVERIFY(query.exec(insertStatement));
    QVERIFY(!query.exec(insertStatement));
}

void TestDatenbank::schema_verhindertDoppeltenLeselisteneintrag()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY(database.createSchema());

    QSqlQuery query(database.connection());
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO user (username, displayName, password_hash, salt, role, is_active) "
        "VALUES ('ma01', 'Max', 'hash', 'salt', 'Employee', 1)")));
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO publication "
        "(arxiv_id, title, authors, summary, arxiv_category, discipline, published_at, url) "
        "VALUES ('2608.01234', 'Title', 'A. Autor', 'Kurzfassung', 'cs.LG', 'ComputerScience', '2026-08-14T09:00:00Z', 'https://arxiv.org/abs/2608.01234')")));

    const QString insertStatement = QStringLiteral(
        "INSERT INTO reading_list_entry (user_id, publication_id, status, rating, note, created_at, changed_at) "
        "VALUES (1, 1, 'Noted', NULL, '', '2026-08-14T09:00:00Z', '2026-08-14T09:00:00Z')");
    QVERIFY(query.exec(insertStatement));
    QVERIFY(!query.exec(insertStatement));
}

void TestDatenbank::schema_loeschtEintraegeMitDerVeroeffentlichung()
{
    Database database(eindeutigerVerbindungsname());
    QVERIFY(database.open(Database::kInMemoryPath));
    QVERIFY(database.createSchema());

    QSqlQuery query(database.connection());
    QVERIFY(query.exec(QStringLiteral("PRAGMA foreign_keys")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);
}

QTEST_GUILESS_MAIN(TestDatenbank)
#include "tst_database.moc"
