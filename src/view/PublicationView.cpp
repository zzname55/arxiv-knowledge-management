#include "view/PublicationView.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "view/SortableTableItem.h"

namespace {
QString toLocalTime(const QDateTime &timestamp)
{
    if (!timestamp.isValid()) {
        return QStringLiteral("—");
    }
    return QLocale::system().toString(timestamp.toLocalTime(), QLocale::ShortFormat);
}
} // namespace

PublicationView::PublicationView(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    buildUi();
    clearDetailView();
}

void PublicationView::buildUi()
{
    m_disciplineSelector = new QComboBox(this);
    m_disciplineSelector->setObjectName(QStringLiteral("disziplinAuswahl"));
    for (const Discipline discipline : allDisciplines()) {
        m_disciplineSelector->addItem(disciplineToText(discipline), QVariant::fromValue(static_cast<int>(discipline)));
    }

    m_refreshButton = new QPushButton(tr("Refresh from arXiv"), this);

    auto *filterleiste = new QHBoxLayout;
    filterleiste->addWidget(new QLabel(tr("Discipline:"), this));
    filterleiste->addWidget(m_disciplineSelector);
    filterleiste->addSpacing(16);
    filterleiste->addWidget(m_refreshButton);
    filterleiste->addStretch();

    m_resultCountLabel = new QLabel(this);
    m_resultCountLabel->setObjectName(QStringLiteral("trefferAnzeige"));
    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName(QStringLiteral("hinweisAnzeige"));
    m_hintLabel->setWordWrap(true);

    m_table = new QTableWidget(0, SpaltenAnzahl, this);
    m_table->setObjectName(QStringLiteral("veroeffentlichungTabelle"));
    m_table->setHorizontalHeaderLabels({ tr("Title"), tr("Discipline"), tr("Published") });
    m_table->horizontalHeader()->setSectionResizeMode(SpalteTitel, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(SpalteDisziplin, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(SpalteDatum, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->horizontalHeader()->setSortIndicatorShown(true);
    m_table->sortByColumn(SpalteDatum, Qt::DescendingOrder);

    m_detailView = new QTextBrowser(this);
    m_detailView->setObjectName(QStringLiteral("detailAnzeige"));
    m_detailView->setOpenExternalLinks(true);

    m_addToReadingListButton = new QPushButton(tr("Add to reading list"), this);
    m_addToReadingListButton->setObjectName(QStringLiteral("leselisteKnopf"));
    m_addToReadingListButton->setEnabled(false);

    auto *detailBereich = new QWidget(this);
    auto *detailLayout  = new QVBoxLayout(detailBereich);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->addWidget(m_detailView);
    detailLayout->addWidget(m_addToReadingListButton);

    auto *aufteilung = new QSplitter(Qt::Horizontal, this);
    aufteilung->addWidget(m_table);
    aufteilung->addWidget(detailBereich);
    aufteilung->setStretchFactor(0, 3);
    aufteilung->setStretchFactor(1, 2);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterleiste);
    mainLayout->addWidget(m_resultCountLabel);
    mainLayout->addWidget(m_hintLabel);
    mainLayout->addWidget(aufteilung, 1);

    connect(m_refreshButton, &QPushButton::clicked, this, &PublicationView::fetchRequested);

    connect(m_disciplineSelector, &QComboBox::currentIndexChanged, this, [this](int position) {
        const auto gewaehlte = static_cast<Discipline>(m_disciplineSelector->itemData(position).toInt());
        emit disciplineSelected(gewaehlte);
    });

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &PublicationView::selectionChanged);

    connect(m_addToReadingListButton, &QPushButton::clicked, this, [this]() {
        if (const Publication *publication = selectedPublication()) {
            emit addToReadingListRequested(publication->id());
        }
    });
}

const Publication *PublicationView::selectedPublication() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return nullptr;
    }
    const QTableWidgetItem *ersteZelle = m_table->item(row, SpalteTitel);
    if (ersteZelle == nullptr) {
        return nullptr;
    }
    const int gesuchteId = ersteZelle->data(kDatensatzIdRolle).toInt();
    for (const Publication &publication : m_displayedPublications) {
        if (publication.id() == gesuchteId) {
            return &publication;
        }
    }
    return nullptr;
}

void PublicationView::showPublications(const QList<Publication> &publications)
{
    m_displayedPublications = publications;

    const bool sortierungWarAn = m_table->isSortingEnabled();
    m_table->setSortingEnabled(false);

    m_table->setRowCount(static_cast<int>(publications.size()));

    for (int row = 0; row < publications.size(); ++row) {
        const Publication &publication = publications.at(row);

        auto *titelZelle = new SortableTableItem(publication.title(), publication.title());
        titelZelle->setData(kDatensatzIdRolle, publication.id());
        m_table->setItem(row, SpalteTitel, titelZelle);

        m_table->setItem(row, SpalteDisziplin,
                           new SortableTableItem(disciplineToText(publication.discipline()),
                                                           disciplineToText(publication.discipline())));

        m_table->setItem(row, SpalteDatum,
                           new SortableTableItem(toLocalTime(publication.publishedAt()),
                                                           publication.publishedAt()));
    }

    m_table->setSortingEnabled(sortierungWarAn);
    clearDetailView();
}

void PublicationView::showResultCount(int displayed, int total)
{
    m_resultCountLabel->setText(tr("Showing: %1 of %2 publications").arg(displayed).arg(total));
}

void PublicationView::showHint(const QString &message)
{
    m_hintLabel->setStyleSheet(QStringLiteral("color: #555555;"));
    m_hintLabel->setText(message);
}

void PublicationView::showError(const QString &message)
{
    m_hintLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_hintLabel->setText(message);
}

void PublicationView::showLoadingIndicator(bool visible)
{
    m_refreshButton->setEnabled(!visible);
    m_refreshButton->setText(visible ? tr("Loading …") : tr("Refresh from arXiv"));

    if (visible) {
        m_hintLabel->setStyleSheet(QStringLiteral("color: #555555;"));
        m_hintLabel->setText(tr("Fetching publications from arXiv …"));
    }
}

void PublicationView::selectionChanged()
{
    const Publication *publication = selectedPublication();
    if (publication == nullptr) {
        clearDetailView();
        return;
    }
    showDetails(*publication);
}

void PublicationView::clearDetailView()
{
    m_detailView->setHtml(QStringLiteral("<p style='color:#777777'>%1</p>").arg(tr("Please select a publication.")));
    m_addToReadingListButton->setEnabled(false);
}

void PublicationView::showDetails(const Publication &publication)
{
    const QString html = QStringLiteral(
        "<h3>%1</h3>"
        "<p><b>%2</b><br>%3</p>"
        "<p><b>%4</b> %5<br><b>%6</b> %7<br><b>%8</b> %9</p>"
        "<p><b>%10</b><br>%11</p>"
        "<p><a href=\"%12\">%12</a></p>")
        .arg(publication.title().toHtmlEscaped(),
             tr("Authors"),
             publication.authorsAsText().toHtmlEscaped(),
             tr("Discipline:"),
             disciplineToText(publication.discipline()),
             tr("Published:"),
             toLocalTime(publication.publishedAt()),
             tr("arXiv ID:"),
             publication.arxivId().toHtmlEscaped())
        .arg(tr("Summary"),
             publication.summary().toHtmlEscaped(),
             publication.url().toHtmlEscaped());

    m_detailView->setHtml(html);
    m_addToReadingListButton->setEnabled(true);
}
