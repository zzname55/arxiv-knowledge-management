#include "model/Discipline.h"

#include <QHash>
#include <QStringList>
#include <QUrlQuery>

namespace {

const QString kApiAdresse = QStringLiteral("https://export.arxiv.org/api/query");

// arXiv does not run "Physics" as a single archive -- it is twelve separate
// archives (astro-ph, cond-mat, gr-qc, hep-ex, hep-lat, hep-ph, hep-th,
// math-ph, nlin, nucl-ex, nucl-th, physics, quant-ph). The original single
// "Physics" discipline is split here into six real subfields instead of
// bundling them behind one artificial umbrella term.
const QHash<QString, Discipline> &prefixMapping()
{
    static const QHash<QString, Discipline> zuordnung = {
        { QStringLiteral("cs"),        Discipline::ComputerScience },
        { QStringLiteral("math"),      Discipline::Mathematics },
        { QStringLiteral("astro-ph"),  Discipline::Astrophysics },
        { QStringLiteral("cond-mat"),  Discipline::CondensedMatterPhysics },
        { QStringLiteral("hep-ex"),    Discipline::HighEnergyPhysics },
        { QStringLiteral("hep-lat"),   Discipline::HighEnergyPhysics },
        { QStringLiteral("hep-ph"),    Discipline::HighEnergyPhysics },
        { QStringLiteral("hep-th"),    Discipline::HighEnergyPhysics },
        { QStringLiteral("quant-ph"),  Discipline::QuantumPhysicsAndGravitation },
        { QStringLiteral("gr-qc"),     Discipline::QuantumPhysicsAndGravitation },
        { QStringLiteral("math-ph"),   Discipline::MathematicalPhysics },
        { QStringLiteral("nlin"),      Discipline::MathematicalPhysics },
        { QStringLiteral("physics"),   Discipline::GeneralPhysics },
        { QStringLiteral("nucl-ex"),   Discipline::GeneralPhysics },
        { QStringLiteral("nucl-th"),   Discipline::GeneralPhysics },
        { QStringLiteral("stat"),      Discipline::Statistics },
        { QStringLiteral("q-bio"),     Discipline::QuantitativeBiology },
        { QStringLiteral("q-fin"),     Discipline::Economics },
        { QStringLiteral("econ"),      Discipline::Economics },
        { QStringLiteral("eess"),      Discipline::ElectricalEngineering }
    };
    return zuordnung;
}

QStringList searchPrefixes(Discipline discipline)
{
    switch (discipline) {
    case Discipline::ComputerScience:
        return { QStringLiteral("cs") };
    case Discipline::Mathematics:
        return { QStringLiteral("math") };
    case Discipline::Astrophysics:
        return { QStringLiteral("astro-ph") };
    case Discipline::CondensedMatterPhysics:
        return { QStringLiteral("cond-mat") };
    case Discipline::HighEnergyPhysics:
        return { QStringLiteral("hep-ex"), QStringLiteral("hep-lat"),
                 QStringLiteral("hep-ph"), QStringLiteral("hep-th") };
    case Discipline::QuantumPhysicsAndGravitation:
        return { QStringLiteral("quant-ph"), QStringLiteral("gr-qc") };
    case Discipline::MathematicalPhysics:
        return { QStringLiteral("math-ph"), QStringLiteral("nlin") };
    case Discipline::GeneralPhysics:
        return { QStringLiteral("physics"), QStringLiteral("nucl-ex"), QStringLiteral("nucl-th") };
    case Discipline::Statistics:
        return { QStringLiteral("stat") };
    case Discipline::QuantitativeBiology:
        return { QStringLiteral("q-bio") };
    case Discipline::Economics:
        return { QStringLiteral("econ"), QStringLiteral("q-fin") };
    case Discipline::ElectricalEngineering:
        return { QStringLiteral("eess") };
    case Discipline::Alle:
    case Discipline::Other:
        break;
    }

    QStringList allePraefixe;
    for (const Discipline fachgebiet : allDisciplines()) {
        if (fachgebiet != Discipline::Alle && fachgebiet != Discipline::Other) {
            allePraefixe += searchPrefixes(fachgebiet);
        }
    }
    return allePraefixe;
}

} // namespace

QString disciplineToText(Discipline discipline)
{
    switch (discipline) {
    case Discipline::Alle:                          return QStringLiteral("All disciplines");
    case Discipline::ComputerScience:                return QStringLiteral("Computer Science");
    case Discipline::Mathematics:                    return QStringLiteral("Mathematics");
    case Discipline::Astrophysics:                   return QStringLiteral("Astrophysics");
    case Discipline::CondensedMatterPhysics:         return QStringLiteral("Condensed Matter");
    case Discipline::HighEnergyPhysics:              return QStringLiteral("High-Energy Physics");
    case Discipline::QuantumPhysicsAndGravitation:   return QStringLiteral("Quantum Physics & Gravitation");
    case Discipline::MathematicalPhysics:            return QStringLiteral("Mathematical Physics");
    case Discipline::GeneralPhysics:                 return QStringLiteral("General Physics");
    case Discipline::Statistics:                     return QStringLiteral("Statistics");
    case Discipline::QuantitativeBiology:            return QStringLiteral("Quantitative Biology");
    case Discipline::Economics:                      return QStringLiteral("Economics");
    case Discipline::ElectricalEngineering:          return QStringLiteral("Electrical Engineering");
    case Discipline::Other:                          return QStringLiteral("Other");
    }
    return QString();
}

std::optional<Discipline> disciplineFromText(const QString &text)
{
    for (const Discipline discipline : allDisciplines()) {
        if (disciplineToText(discipline) == text) {
            return discipline;
        }
    }
    return std::nullopt;
}

QList<Discipline> allDisciplines()
{
    return { Discipline::Alle, Discipline::ComputerScience, Discipline::Mathematics,
             Discipline::Astrophysics, Discipline::CondensedMatterPhysics, Discipline::HighEnergyPhysics,
             Discipline::QuantumPhysicsAndGravitation, Discipline::MathematicalPhysics, Discipline::GeneralPhysics,
             Discipline::Statistics, Discipline::QuantitativeBiology,
             Discipline::Economics, Discipline::ElectricalEngineering, Discipline::Other };
}

Discipline disciplineFromArxivCategory(const QString &arxivCategory)
{
    if (arxivCategory.isEmpty()) {
        return Discipline::Other;
    }
    const QString praefix = arxivCategory.section(QLatin1Char('.'), 0, 0);
    return prefixMapping().value(praefix, Discipline::Other);
}

QUrl buildArxivQueryUrl(Discipline discipline, int maxResults)
{
    QStringList categoryExpressions;
    for (const QString &prefix : searchPrefixes(discipline)) {
        // No dot before the wildcard: some archives (e.g. "quant-ph", "gr-qc",
        // "hep-th") have no subcategories on arXiv and contain no dot
        // themselves. "cat:quant-ph.*" therefore never matched anything --
        // that is why those disciplines returned no results.
        categoryExpressions.append(QStringLiteral("cat:%1*").arg(prefix));
    }

    QUrlQuery queryParts;
    queryParts.addQueryItem(QStringLiteral("search_query"), categoryExpressions.join(QStringLiteral(" OR ")));
    queryParts.addQueryItem(QStringLiteral("sortBy"),      QStringLiteral("submittedDate"));
    queryParts.addQueryItem(QStringLiteral("sortOrder"),   QStringLiteral("descending"));
    queryParts.addQueryItem(QStringLiteral("start"),       QStringLiteral("0"));
    queryParts.addQueryItem(QStringLiteral("max_results"), QString::number(maxResults));

    QUrl url(kApiAdresse);
    url.setQuery(queryParts);
    return url;
}
