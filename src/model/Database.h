// ---------------------------------------------------------------------------
// Database — MVC layer: MODEL
// Kapselt die SQLite-Verbindung und legt das Schema an.
// ---------------------------------------------------------------------------
#pragma once

#include <QSqlDatabase>
#include <QString>

class Database
{
public:
    explicit Database(QString verbindungsname);
    ~Database();

    Database(const Database &)            = delete;
    Database &operator=(const Database &) = delete;

    bool open(const QString &dateipfad);
    bool isOpen() const;
    QSqlDatabase connection() const;
    bool createSchema();
    QString lastError() const;

    static const QString kInMemoryPath;

private:
    bool execute(const QString &sqlAnweisung);

    QString m_connectionName;
    QString m_lastError;
};
