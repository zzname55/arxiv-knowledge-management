#include "view/CompleteReadingDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "model/ReadingListService.h"

CompleteReadingDialog::CompleteReadingDialog(const QString &title, QWidget *parentWidget)
    : QDialog(parentWidget)
{
    buildUi(title);
}

void CompleteReadingDialog::buildUi(const QString &title)
{
    setWindowTitle(tr("Complete Reading"));
    setModal(true);

    auto *titleLabel = new QLabel(tr("Publication"), this);
    auto *titelAnzeige     = new QLabel(title, this);
    titelAnzeige->setWordWrap(true);
    QFont titelSchrift = titelAnzeige->font();
    titelSchrift.setBold(true);
    titelAnzeige->setFont(titelSchrift);

    m_ratingField = new QSpinBox(this);
    m_ratingField->setRange(ReadingListService::kRatingMinimum, ReadingListService::kRatingMaximum);
    m_ratingField->setValue(ReadingListService::kRatingMinimum + 2);
    m_ratingField->setSuffix(tr("   (1 = low, 5 = high)"));

    auto *ratingRow = new QHBoxLayout;
    ratingRow->addWidget(new QLabel(tr("Rating *"), this));
    ratingRow->addWidget(m_ratingField);
    ratingRow->addStretch();

    m_noteField = new QPlainTextEdit(this);
    m_noteField->setPlaceholderText(tr("What is this publication useful for at work?"));
    m_noteField->setMinimumHeight(90);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setMinimumHeight(28);

    auto *cancelButton = new QPushButton(tr("Cancel"), this);
    auto *saveButton = new QPushButton(tr("Save"), this);
    saveButton->setDefault(true);

    auto *buttonBar = new QHBoxLayout;
    buttonBar->addWidget(new QLabel(tr("* Required field"), this));
    buttonBar->addStretch();
    buttonBar->addWidget(cancelButton);
    buttonBar->addWidget(saveButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(titelAnzeige);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(ratingRow);
    mainLayout->addWidget(new QLabel(tr("Note *"), this));
    mainLayout->addWidget(m_noteField);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addLayout(buttonBar);

    setMinimumWidth(460);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &CompleteReadingDialog::validateInputAndAccept);
}

void CompleteReadingDialog::validateInputAndAccept()
{
    if (m_noteField->toPlainText().trimmed().isEmpty()) {
        showError(ReadingListService::kMessageNoteMissing);
        m_noteField->setFocus();
        return;
    }
    accept();
}

int CompleteReadingDialog::rating() const
{
    return m_ratingField->value();
}

QString CompleteReadingDialog::note() const
{
    return m_noteField->toPlainText().trimmed();
}

void CompleteReadingDialog::showError(const QString &message)
{
    m_errorLabel->setText(message);
}
