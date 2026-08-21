// ---------------------------------------------------------------------------
// Test: PasswordHasher (US-01, AK-01.8)
// ---------------------------------------------------------------------------
#include <QtTest>
#include "model/PasswordHasher.h"

class TestPasswortHasher : public QObject
{
    Q_OBJECT

private slots:
    void erzeugeSalt_liefertHexZeichenketteMitFesterLaenge();
    void erzeugeSalt_liefertBeiJedemAufrufEinenAnderenWert();
    void hashe_liefertSha256AlsHexZeichenkette();
    void hashe_enthaeltDasKlartextpasswortNicht();
    void hashe_istBeiGleicherEingabeReproduzierbar();
    void hashe_liefertMitAnderemSaltEinAnderesErgebnis();
    void pruefe_erkenntDasRichtigePasswort();
    void pruefe_lehntEinFalschesPasswortAb();
    void pruefe_lehntAbWennDerSaltNichtPasst();
    void pruefe_lehntEinLeeresPasswortAb();
};

void TestPasswortHasher::erzeugeSalt_liefertHexZeichenketteMitFesterLaenge()
{
    const QString salt = PasswordHasher::generateSalt();
    QCOMPARE(salt.length(), 32);
    QVERIFY(QRegularExpression(QStringLiteral("^[0-9a-f]{32}$")).match(salt).hasMatch());
}

void TestPasswortHasher::erzeugeSalt_liefertBeiJedemAufrufEinenAnderenWert()
{
    QVERIFY(PasswordHasher::generateSalt() != PasswordHasher::generateSalt());
}

void TestPasswortHasher::hashe_liefertSha256AlsHexZeichenkette()
{
    const QString hash = PasswordHasher::hash(QStringLiteral("geheim123"), QStringLiteral("abcdef0123456789abcdef0123456789"));
    QCOMPARE(hash.length(), 64);
    QVERIFY(QRegularExpression(QStringLiteral("^[0-9a-f]{64}$")).match(hash).hasMatch());
}

void TestPasswortHasher::hashe_enthaeltDasKlartextpasswortNicht()
{
    const QString password = QStringLiteral("MeinSuperGeheimesPasswort");
    const QString hash     = PasswordHasher::hash(password, PasswordHasher::generateSalt());
    QVERIFY(!hash.contains(password, Qt::CaseInsensitive));
}

void TestPasswortHasher::hashe_istBeiGleicherEingabeReproduzierbar()
{
    const QString salt = QStringLiteral("0123456789abcdef0123456789abcdef");
    QCOMPARE(PasswordHasher::hash(QStringLiteral("password"), salt), PasswordHasher::hash(QStringLiteral("password"), salt));
}

void TestPasswortHasher::hashe_liefertMitAnderemSaltEinAnderesErgebnis()
{
    const QString a = PasswordHasher::hash(QStringLiteral("password"), QStringLiteral("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    const QString b = PasswordHasher::hash(QStringLiteral("password"), QStringLiteral("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"));
    QVERIFY(a != b);
}

void TestPasswortHasher::pruefe_erkenntDasRichtigePasswort()
{
    const QString salt = PasswordHasher::generateSalt();
    const QString hash = PasswordHasher::hash(QStringLiteral("start1234"), salt);
    QVERIFY(PasswordHasher::verify(QStringLiteral("start1234"), salt, hash));
}

void TestPasswortHasher::pruefe_lehntEinFalschesPasswortAb()
{
    const QString salt = PasswordHasher::generateSalt();
    const QString hash = PasswordHasher::hash(QStringLiteral("start1234"), salt);
    QVERIFY(!PasswordHasher::verify(QStringLiteral("start12345"), salt, hash));
}

void TestPasswortHasher::pruefe_lehntAbWennDerSaltNichtPasst()
{
    const QString hash = PasswordHasher::hash(QStringLiteral("start1234"), QStringLiteral("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    QVERIFY(!PasswordHasher::verify(QStringLiteral("start1234"), QStringLiteral("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"), hash));
}

void TestPasswortHasher::pruefe_lehntEinLeeresPasswortAb()
{
    const QString salt = PasswordHasher::generateSalt();
    const QString hash = PasswordHasher::hash(QString(), salt);
    QVERIFY(!PasswordHasher::verify(QString(), salt, hash));
}

QTEST_APPLESS_MAIN(TestPasswortHasher)
#include "tst_passwordhasher.moc"
