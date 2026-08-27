// ---------------------------------------------------------------------------
// ReadingListEntry — MVC layer: MODEL
// Verbindung zwischen User und Publication: ein Vorgang im Prozess.
// userDisplayName/publicationTitle kommen aus verknuepften Tabellen,
// nur zur Anzeige, werden beim Schreiben ignoriert.
// ---------------------------------------------------------------------------
#pragma once

#include <QDateTime>
#include <QString>
#include <optional>
#include "model/ReadingStatus.h"

class ReadingListEntry
{
public:
    ReadingListEntry() = default;

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int userId() const { return m_benutzerId; }
    void setUserId(int userId) { m_benutzerId = userId; }

    int publicationId() const { return m_veroeffentlichungId; }
    void setPublicationId(int id) { m_veroeffentlichungId = id; }

    ReadingStatus status() const { return m_status; }
    void setStatus(ReadingStatus status) { m_status = status; }

    std::optional<int> rating() const { return m_bewertung; }
    void setRating(std::optional<int> rating) { m_bewertung = rating; }

    QString note() const { return m_notiz; }
    void setNote(const QString &note) { m_notiz = note; }

    QDateTime createdAt() const { return m_erstelltAm; }
    void setCreatedAt(const QDateTime &timestamp) { m_erstelltAm = timestamp; }

    QDateTime changedAt() const { return m_geaendertAm; }
    void setChangedAt(const QDateTime &timestamp) { m_geaendertAm = timestamp; }

    QString userDisplayName() const { return m_benutzerAnzeigename; }
    void setUserDisplayName(const QString &displayName) { m_benutzerAnzeigename = displayName; }

    QString publicationTitle() const { return m_veroeffentlichungTitel; }
    void setPublicationTitle(const QString &title) { m_veroeffentlichungTitel = title; }

    bool isPersisted() const { return m_id != kNoId; }

    QString ratingAsText() const
    {
        return m_bewertung.has_value() ? QString::number(*m_bewertung) : QStringLiteral("—");
    }

    static constexpr int kNoId = 0;

private:
    int                m_id                  = kNoId;
    int                m_benutzerId          = 0;
    int                m_veroeffentlichungId = 0;
    ReadingStatus         m_status              = ReadingStatus::Noted;
    std::optional<int> m_bewertung;
    QString            m_notiz;
    QDateTime          m_erstelltAm;
    QDateTime          m_geaendertAm;
    QString            m_benutzerAnzeigename;
    QString            m_veroeffentlichungTitel;
};
