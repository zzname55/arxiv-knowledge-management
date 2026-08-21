#include "model/Discipline.h"

#include <QHash>
#include <QStringList>
#include <QUrlQuery>

namespace {

const QString kApiAddress = QStringLiteral("https://export.arxiv.org/api/query");

const QHash<QString, Discipline> &prefixMapping()
{
    static const QHash<QString, Discipline> zuordnung = {
        { QStringLiteral("cs"),        Discipline::ComputerScience },
        { QStringLiteral("math"),      Discipline::Mathematics },
        { QStringLiteral("physics"),   Discipline::Physics },
        { QStringLiteral("astro-ph"),  Discipline::Physics },
        { QStringLiteral("cond-mat"),  Discipline::Physics },
        { QStringLiteral("gr-qc"),     Discipline::Physics },
        { QStringLiteral("hep-ex"),    Discipline::Physics },
        { QStringLiteral("hep-lat"),   Discipline::Physics },
        { QStringLiteral("hep-ph"),    Discipline::Physics },
        { QStringLiteral("hep-th"),    Discipline::Physics },
        { QStringLiteral("math-ph"),   Discipline::Physics },
        { QStringLiteral("nlin"),      Discipline::Physics },
        { QStringLiteral("nucl-ex"),   Discipline::Physics },
        { QStringLiteral("nucl-th"),   Discipline::Physics },
        { QStringLiteral("quant-ph"),  Discipline::Physics },
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
    case Discipline::Physics:
        return { QStringLiteral("physics"), QStringLiteral("astro-ph"),
                 QStringLiteral("cond-mat"), QStringLiteral("quant-ph"),
                 QStringLiteral("hep-th"), QStringLiteral("gr-qc") };
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
    case Discipline::Alle:                      return QStringLiteral("All disciplines");
    case Discipline::ComputerScience:                return QStringLiteral("ComputerScience");
    case Discipline::Mathematics:                return QStringLiteral("Mathematics");
    case Discipline::Physics:                    return QStringLiteral("Physics");
    case Discipline::Statistics:                 return QStringLiteral("Statistics");
    case Discipline::QuantitativeBiology:      return QStringLiteral("Quantitative Biologie");
    case Discipline::Economics: return QStringLiteral("Economics");
    case Discipline::ElectricalEngineering:            return QStringLiteral("ElectricalEngineering");
    case Discipline::Other:                  return QStringLiteral("Other");
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
    return { Discipline::Alle, Discipline::ComputerScience, Discipline::Mathematics, Discipline::Physics,
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

QUrl buildArxivQueryUrl(Discipline discipline, int maxTreffer)
{
    QStringList kategorieAusdruecke;
    for (const QString &praefix : searchPrefixes(discipline)) {
        kategorieAusdruecke.append(QStringLiteral("cat:%1.*").arg(praefix));
    }

    QUrlQuery abfrageteile;
    abfrageteile.addQueryItem(QStringLiteral("search_query"), kategorieAusdruecke.join(QStringLiteral(" OR ")));
    abfrageteile.addQueryItem(QStringLiteral("sortBy"),      QStringLiteral("submittedDate"));
    abfrageteile.addQueryItem(QStringLiteral("sortOrder"),   QStringLiteral("descending"));
    abfrageteile.addQueryItem(QStringLiteral("start"),       QStringLiteral("0"));
    abfrageteile.addQueryItem(QStringLiteral("max_results"), QString::number(maxTreffer));

    QUrl url(kApiAddress);
    url.setQuery(abfrageteile);
    return url;
}
