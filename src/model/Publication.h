// ---------------------------------------------------------------------------
// Publication — MVC layer: MODEL
// Eine wissenschaftliche Arbeit aus arXiv. arXiv-ID ist der fachliche
// Schluessel fuer den Duplikatschutz (AK-02.6).
// ---------------------------------------------------------------------------
#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include "model/Discipline.h"

class Publication
{
public:
    Publication() = default;

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString arxivId() const { return m_arxivId; }
    void setArxivId(const QString &arxivId) { m_arxivId = arxivId; }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QStringList authors() const { return m_autoren; }
    void setAuthors(const QStringList &authors) { m_autoren = authors; }

    QString summary() const { return m_zusammenfassung; }
    void setSummary(const QString &summary) { m_zusammenfassung = summary; }

    QString arxivCategory() const { return m_arxivKategorie; }
    void setArxivCategory(const QString &kategorie) { m_arxivKategorie = kategorie; }

    Discipline discipline() const { return m_discipline; }
    void setDiscipline(Discipline discipline) { m_discipline = discipline; }

    QDateTime publishedAt() const { return m_veroeffentlichtAm; }
    void setPublishedAt(const QDateTime &timestamp) { m_veroeffentlichtAm = timestamp; }

    QString url() const { return m_url; }
    void setUrl(const QString &url) { m_url = url; }

    bool isPersisted() const { return m_id != kNoId; }
    QString authorsAsText() const { return m_autoren.join(QStringLiteral(", ")); }

    static constexpr int kNoId = 0;

private:
    int         m_id = kNoId;
    QString     m_arxivId;
    QString     m_title;
    QStringList m_autoren;
    QString     m_zusammenfassung;
    QString     m_arxivKategorie;
    Discipline   m_discipline = Discipline::Other;
    QDateTime   m_veroeffentlichtAm;
    QString     m_url;
};
