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
            publication.setArxivId(idFromAddress(leser.readElementText()));
        } else if (elementname == kElementTitle) {
            publication.setTitle(leser.readElementText().simplified());
        } else if (elementname == kElementSummary) {
            publication.setSummary(leser.readElementText().simplified());
        } else if (elementname == kElementPublished) {
            publication.setPublishedAt(QDateTime::fromString(leser.readElementText(), Qt::ISODate));
        } else if (elementname == kElementPrimaryCategory) {
            const QString kategorie = leser.attributes().value(kAttributeCategory).toString();
            publication.setArxivCategory(kategorie);
            publication.setDiscipline(disciplineFromArxivCategory(kategorie));
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

    publication.setAuthors(authors);

    if (!publication.arxivId().isEmpty()) {
        publication.setUrl(kArxivPagePrefix + publication.arxivId());
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
            *failure = QStringLiteral("arXiv returned an empty response. Please try again later.");
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
            *failure = QStringLiteral("The response from arXiv could not be parsed (row %1): %2")
                          .arg(leser.lineNumber())
                          .arg(leser.errorString());
        }
        return {};
    }

    return publications;
}
