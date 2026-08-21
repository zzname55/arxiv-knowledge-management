#include "model/ArxivAtomParser.h"

#include <QRegularExpression>
#include <QXmlStreamReader>

namespace {

const QLatin1String kElementEntry("entry");
const QLatin1String kElementId("id");
const QLatin1String kElementTitle("title");
const QLatin1String kElementSummary("summary");
const QLatin1String kElementPublished("published");
const QLatin1String kElementAuthor("author");
const QLatin1String kElementName("name");
const QLatin1String kElementPrimaryCategory("primary_category");
const QLatin1String kAttributeCategory("term");

const QString kArxivPagePrefix = QStringLiteral("https://arxiv.org/abs/");

QString idFromAddress(const QString &adresse)
{
    const int trennerPosition = adresse.lastIndexOf(QLatin1String("/abs/"));
    if (trennerPosition < 0) {
        return QString();
    }
    QString kennung = adresse.mid(trennerPosition + 5);
    static const QRegularExpression versionsanhang(QStringLiteral("v\\d+$"));
    kennung.remove(versionsanhang);
    return kennung;
}

Publication readEntry(QXmlStreamReader &leser)
{
    Publication publication;
    QStringList       authors;

    while (!leser.atEnd()) {
        leser.readNext();

        if (leser.isEndElement() && leser.name() == kElementEntry) {
            break;
        }
        if (!leser.isStartElement()) {
            continue;
        }

        const QStringView elementname = leser.name();

        if (elementname == kElementId) {
            publication.setzeArxivId(idFromAddress(leser.readElementText()));
        } else if (elementname == kElementTitle) {
            publication.setzeTitel(leser.readElementText().simplified());
        } else if (elementname == kElementSummary) {
            publication.setzeZusammenfassung(leser.readElementText().simplified());
        } else if (elementname == kElementPublished) {
            publication.setzeVeroeffentlichtAm(QDateTime::fromString(leser.readElementText(), Qt::ISODate));
        } else if (elementname == kElementPrimaryCategory) {
            const QString kategorie = leser.attributes().value(kAttributeCategory).toString();
            publication.setzeArxivKategorie(kategorie);
            publication.setzeDisziplin(disciplineFromArxivCategory(kategorie));
        } else if (elementname == kElementAuthor) {
            while (!leser.atEnd()) {
                leser.readNext();
                if (leser.isEndElement() && leser.name() == kElementAuthor) {
                    break;
                }
                if (leser.isStartElement() && leser.name() == kElementName) {
                    authors.append(leser.readElementText().simplified());
                }
            }
        }
    }

    publication.setzeAutoren(authors);

    if (!publication.arxivId().isEmpty()) {
        publication.setzeUrl(kArxivPagePrefix + publication.arxivId());
    }

    return publication;
}

} // namespace

QList<Publication> ArxivAtomParser::parse(const QByteArray &atomAntwort, QString *failure)
{
    if (failure != nullptr) {
        failure->clear();
    }

    QList<Publication> publications;

    if (atomAntwort.isEmpty()) {
        if (failure != nullptr) {
            *failure = QStringLiteral("Von arXiv kam eine leere Antwort. Bitte spaeter erneut versuchen.");
        }
        return publications;
    }

    QXmlStreamReader leser(atomAntwort);

    while (!leser.atEnd()) {
        leser.readNext();

        if (leser.isStartElement() && leser.name() == kElementEntry) {
            const Publication publication = readEntry(leser);
            if (!publication.arxivId().isEmpty()) {
                publications.append(publication);
            }
        }
    }

    if (leser.hasError()) {
        if (failure != nullptr) {
            *failure = QStringLiteral("Die Antwort von arXiv konnte nicht ausgewertet werden (row %1): %2")
                          .arg(leser.lineNumber())
                          .arg(leser.errorString());
        }
        return {};
    }

    return publications;
}
