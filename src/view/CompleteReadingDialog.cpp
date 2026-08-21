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
    setWindowTitle(tr("Lesen abschließen"));
    setModal(true);

    auto *titelBezeichnung = new QLabel(tr("Veröffentlichung"), this);
    auto *titelAnzeige     = new QLabel(title, this);
    titelAnzeige->setWordWrap(true);
    QFont titelSchrift = titelAnzeige->font();
    titelSchrift.setBold(true);
    titelAnzeige->setFont(titelSchrift);

    m_bewertungFeld = new QSpinBox(this);
    m_bewertungFeld->setRange(ReadingListService::kRatingMinimum, ReadingListService::kRatingMaximum);
    m_bewertungFeld->setValue(ReadingListService::kRatingMinimum + 2);
    m_bewertungFeld->setSuffix(tr("   (1 = gering, 5 = hoch)"));

    auto *bewertungZeile = new QHBoxLayout;
    bewertungZeile->addWidget(new QLabel(tr("Rating *"), this));
    bewertungZeile->addWidget(m_bewertungFeld);
    bewertungZeile->addStretch();

    m_notizFeld = new QPlainTextEdit(this);
    m_notizFeld->setPlaceholderText(tr("Wofür ist diese Veröffentlichung im Betrieb nützlich?"));
    m_notizFeld->setMinimumHeight(90);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setMinimumHeight(28);

    auto *abbrechenKnopf = new QPushButton(tr("Abbrechen"), this);
    auto *speichernKnopf = new QPushButton(tr("Speichern"), this);
    speichernKnopf->setDefault(true);

    auto *knopfleiste = new QHBoxLayout;
    knopfleiste->addWidget(new QLabel(tr("* Pflichtfeld"), this));
    knopfleiste->addStretch();
    knopfleiste->addWidget(abbrechenKnopf);
    knopfleiste->addWidget(speichernKnopf);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titelBezeichnung);
    mainLayout->addWidget(titelAnzeige);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(bewertungZeile);
    mainLayout->addWidget(new QLabel(tr("Note *"), this));
    mainLayout->addWidget(m_notizFeld);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addLayout(knopfleiste);

    setMinimumWidth(460);

    connect(abbrechenKnopf, &QPushButton::clicked, this, &QDialog::reject);
    connect(speichernKnopf, &QPushButton::clicked, this, &CompleteReadingDialog::validateInputAndAccept);
}

void CompleteReadingDialog::validateInputAndAccept()
{
    if (m_notizFeld->toPlainText().trimmed().isEmpty()) {
        showError(ReadingListService::kMessageNoteMissing);
        m_notizFeld->setFocus();
        return;
    }
    accept();
}

int CompleteReadingDialog::rating() const
{
    return m_bewertungFeld->value();
}

QString CompleteReadingDialog::note() const
{
    return m_notizFeld->toPlainText().trimmed();
}

void CompleteReadingDialog::showError(const QString &message)
{
    m_errorLabel->setText(message);
}
