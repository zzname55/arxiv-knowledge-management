#include "view/ReadingListView.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "view/CompleteReadingDialog.h"
#include "view/SortableTableItem.h"

namespace {
QString toLocalTime(const QDateTime &timestamp)
{
    if (!timestamp.isValid()) {
        return QStringLiteral("—");
    }
    return QLocale::system().toString(timestamp.toLocalTime().date(), QLocale::ShortFormat);
}
} // namespace

ReadingListView::ReadingListView(Mode mode, QWidget *parentWidget)
    : QWidget(parentWidget)
    , m_mode(mode)
{
    buildUi();
    selectionChanged();
}

void ReadingListView::buildUi()
{
    const bool allLists = (m_mode == Mode::AllLists);

    auto *heading = new QLabel(allLists ? tr("Approvals — all reading lists") : tr("My Reading List"), this);
    QFont ueberschriftSchrift = heading->font();
    ueberschriftSchrift.setPointSize(ueberschriftSchrift.pointSize() + 2);
    ueberschriftSchrift.setBold(true);
    heading->setFont(ueberschriftSchrift);

    QStringList columnHeaders;
    columnHeaders << tr("Title");
    if (allLists) {
        columnHeaders << tr("User");
    }
    columnHeaders << tr("Status") << tr("Rating") << tr("Since");

    m_table = new QTableWidget(0, static_cast<int>(columnHeaders.size()), this);
    m_table->setObjectName(QStringLiteral("leselisteTabelle"));
    m_table->setHorizontalHeaderLabels(columnHeaders);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int column = 1; column < columnHeaders.size(); ++column) {
        m_table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
    }
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->horizontalHeader()->setSortIndicatorShown(true);

    m_noteView = new QTextBrowser(this);
    m_noteView->setMaximumHeight(90);

    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName(QStringLiteral("meldungsAnzeige"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setMinimumHeight(28);

    auto *buttonBar = new QHBoxLayout;

    if (allLists) {
        m_approveButton   = new QPushButton(tr("Approve for training"), this);
        m_approveButton->setObjectName(QStringLiteral("freigebenKnopf"));
        m_archiveButton = new QPushButton(tr("Archive"), this);
        m_archiveButton->setObjectName(QStringLiteral("archivierenKnopf"));
        buttonBar->addWidget(m_approveButton);
        buttonBar->addWidget(m_archiveButton);
    } else {
        m_startReadingButton = new QPushButton(tr("Start reading"), this);
        m_startReadingButton->setObjectName(QStringLiteral("lesenBeginnenKnopf"));
        m_completeButton  = new QPushButton(tr("Mark as read"), this);
        m_completeButton->setObjectName(QStringLiteral("abschliessenKnopf"));
        m_discardButton     = new QPushButton(tr("Discard"), this);
        m_discardButton->setObjectName(QStringLiteral("verwerfenKnopf"));
        buttonBar->addWidget(m_startReadingButton);
        buttonBar->addWidget(m_completeButton);
        buttonBar->addWidget(m_discardButton);
    }
    buttonBar->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(heading);
    mainLayout->addWidget(m_table, 1);
    mainLayout->addWidget(new QLabel(tr("Reader's note:"), this));
    mainLayout->addWidget(m_noteView);
    mainLayout->addLayout(buttonBar);
    mainLayout->addWidget(m_messageLabel);

    if (!allLists) {
        auto *hint = new QLabel(tr("Hint: approval for training is done by the Knowledge Manager."), this);
        hint->setStyleSheet(QStringLiteral("color: #777777;"));
        mainLayout->addWidget(hint);
    }

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &ReadingListView::selectionChanged);

    const auto connectToSelection = [this](QPushButton *button, auto signal) {
        if (button == nullptr) {
            return;
        }
        connect(button, &QPushButton::clicked, this, [this, signal]() {
            if (const ReadingListEntry *entry = selectedEntry()) {
                emit (this->*signal)(entry->id());
            }
        });
    };

    connectToSelection(m_startReadingButton, &ReadingListView::startReadingRequested);
    connectToSelection(m_discardButton,     &ReadingListView::discardRequested);
    connectToSelection(m_approveButton,     &ReadingListView::approveRequested);
    connectToSelection(m_archiveButton,   &ReadingListView::archiveRequested);

    if (m_completeButton != nullptr) {
        connect(m_completeButton, &QPushButton::clicked, this, &ReadingListView::openCompletionDialog);
    }
}

const ReadingListEntry *ReadingListView::selectedEntry() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return nullptr;
    }
    const QTableWidgetItem *firstCell = m_table->item(row, 0);
    if (firstCell == nullptr) {
        return nullptr;
    }
    const int wantedId = firstCell->data(kDatensatzIdRolle).toInt();
    for (const ReadingListEntry &entry : m_displayedEntries) {
        if (entry.id() == wantedId) {
            return &entry;
        }
    }
    return nullptr;
}

void ReadingListView::showEntries(const QList<ReadingListEntry> &entries)
{
    m_displayedEntries = entries;
    const bool allLists = (m_mode == Mode::AllLists);

    const bool sortingWasEnabled = m_table->isSortingEnabled();
    m_table->setSortingEnabled(false);

    m_table->setRowCount(static_cast<int>(entries.size()));

    for (int row = 0; row < entries.size(); ++row) {
        const ReadingListEntry &entry = entries.at(row);
        int column = 0;

        auto *titleCell = new SortableTableItem(entry.publicationTitle(), entry.publicationTitle());
        titleCell->setData(kDatensatzIdRolle, entry.id());
        m_table->setItem(row, column++, titleCell);

        if (allLists) {
            m_table->setItem(row, column++,
                               new SortableTableItem(entry.userDisplayName(), entry.userDisplayName()));
        }

        m_table->setItem(row, column++,
                           new SortableTableItem(readingStatusToText(entry.status()),
                                                           static_cast<int>(allReadingStatuses().indexOf(entry.status()))));

        m_table->setItem(row, column++,
                           new SortableTableItem(entry.ratingAsText(), entry.rating().value_or(-1)));

        m_table->setItem(row, column++,
                           new SortableTableItem(toLocalTime(entry.createdAt()), entry.createdAt()));
    }

    m_table->setSortingEnabled(sortingWasEnabled);
    selectionChanged();
}

void ReadingListView::selectionChanged()
{
    const ReadingListEntry *entry = selectedEntry();

    m_noteView->setPlainText(entry != nullptr && !entry->note().isEmpty() ? entry->note() : tr("—"));

    const ReadingStatus status    = entry != nullptr ? entry->status() : ReadingStatus::Archived;
    const bool       hasSelection = (entry != nullptr);

    if (m_startReadingButton != nullptr) {
        m_startReadingButton->setEnabled(hasSelection && status == ReadingStatus::Noted);
    }
    if (m_completeButton != nullptr) {
        m_completeButton->setEnabled(hasSelection && status == ReadingStatus::InProgress);
    }
    if (m_discardButton != nullptr) {
        m_discardButton->setEnabled(hasSelection && (status == ReadingStatus::Noted || status == ReadingStatus::InProgress));
    }
    if (m_approveButton != nullptr) {
        m_approveButton->setEnabled(hasSelection && status == ReadingStatus::Read);
    }
    if (m_archiveButton != nullptr) {
        m_archiveButton->setEnabled(hasSelection && status == ReadingStatus::ApprovedForTraining);
    }
}

void ReadingListView::openCompletionDialog()
{
    const ReadingListEntry *entry = selectedEntry();
    if (entry == nullptr) {
        return;
    }
    const int entryId = entry->id();

    CompleteReadingDialog dialog(entry->publicationTitle(), this);
    if (dialog.exec() == QDialog::Accepted) {
        emit completeReadingRequested(entryId, dialog.rating(), dialog.note());
    }
}

void ReadingListView::showMessage(const QString &message)
{
    m_messageLabel->setStyleSheet(QStringLiteral("color: #1b5e20;"));
    m_messageLabel->setText(message);
}

void ReadingListView::showError(const QString &message)
{
    m_messageLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_messageLabel->setText(message);
}
