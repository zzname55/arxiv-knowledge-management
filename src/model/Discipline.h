// ---------------------------------------------------------------------------
// Discipline — MVC layer: MODEL
// Wissenschaftliche Fachgebiete + Zuordnung der arXiv-Kategorien + Abfrage-URL.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include <QUrl>
#include <optional>

enum class Discipline {
    Alle,
    ComputerScience,
    Mathematics,
    Physics,
    Statistics,
    QuantitativeBiology,
    Economics,
    ElectricalEngineering,
    Other
};

QString disciplineToText(Discipline discipline);
std::optional<Discipline> disciplineFromText(const QString &text);
QList<Discipline> allDisciplines();

/// Ordnet eine arXiv-Kategorie ("cs.LG") der Discipline zu; unbekannt -> Other.
Discipline disciplineFromArxivCategory(const QString &arxivCategory);

/// Baut die Abfrage-URL fuer die arXiv-API, sortiert nach Einreichungsdatum absteigend.
QUrl buildArxivQueryUrl(Discipline discipline, int maxTreffer);

inline constexpr int kDefaultResultCount = 5;
