// ---------------------------------------------------------------------------
// PublicationRepository — MVC layer: MODEL
// Rein virtuelle Schnittstelle zur Datenhaltung der publications.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include <optional>
#include "model/Publication.h"

class PublicationRepository
{
public:
    virtual ~PublicationRepository() = default;

    static constexpr int kUnlimited = -1;

    virtual std::optional<Publication> findById(int id) const = 0;
    virtual std::optional<Publication> findByArxivId(const QString &arxivId) const = 0;
    virtual QList<Publication> findByDiscipline(Discipline discipline, int maxCount = kUnlimited) const = 0;
    virtual bool save(Publication &publication) = 0;
    virtual int count() const = 0;
};
