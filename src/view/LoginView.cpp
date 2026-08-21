#include "view/LoginView.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginView::LoginView(QWidget *parentWidget)
    : QDialog(parentWidget)
{
    buildUi();
}

void LoginView::buildUi()
{
    setWindowTitle(tr("ArxivKnowledgeManagement — Login"));
    setModal(true);

    auto *heading = new QLabel(tr("Wissensmanagement TechnoLab"), this);
    QFont ueberschriftSchrift = heading->font();
    ueberschriftSchrift.setPointSize(ueberschriftSchrift.pointSize() + 4);
    ueberschriftSchrift.setBold(true);
    heading->setFont(ueberschriftSchrift);
    heading->setAlignment(Qt::AlignCenter);

    auto *trennlinie = new QFrame(this);
    trennlinie->setFrameShape(QFrame::HLine);
    trennlinie->setFrameShadow(QFrame::Sunken);

    m_usernameField = new QLineEdit(this);
    m_usernameField->setObjectName(QStringLiteral("benutzernameFeld"));
    m_usernameField->setPlaceholderText(tr("z. B. ma01"));

    m_passwordField = new QLineEdit(this);
    m_passwordField->setObjectName(QStringLiteral("passwortFeld"));
    m_passwordField->setEchoMode(QLineEdit::Password);

    auto *formular = new QFormLayout;
    formular->addRow(tr("Username"), m_usernameField);
    formular->addRow(tr("Password"),     m_passwordField);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("fehlerAnzeige"));
    m_errorLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setMinimumHeight(32);

    auto *abbrechenKnopf = new QPushButton(tr("Abbrechen"), this);
    m_loginButton       = new QPushButton(tr("Anmelden"), this);
    m_loginButton->setObjectName(QStringLiteral("anmeldenKnopf"));
    m_loginButton->setDefault(true);

    auto *knopfleiste = new QHBoxLayout;
    knopfleiste->addStretch();
    knopfleiste->addWidget(abbrechenKnopf);
    knopfleiste->addWidget(m_loginButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(heading);
    mainLayout->addWidget(trennlinie);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(formular);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addLayout(knopfleiste);

    setMinimumWidth(380);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginView::loginRequested);
    connect(abbrechenKnopf, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_usernameField, &QLineEdit::returnPressed, this, &LoginView::loginRequested);
    connect(m_passwordField, &QLineEdit::returnPressed, this, &LoginView::loginRequested);
}

QString LoginView::username() const
{
    return m_usernameField->text().trimmed();
}

QString LoginView::password() const
{
    return m_passwordField->text();
}

void LoginView::showError(const QString &message)
{
    m_errorLabel->setText(message);
    m_passwordField->clear();
    m_passwordField->setFocus();
}

void LoginView::clearErrorDisplay()
{
    m_errorLabel->clear();
}

void LoginView::loginCompleted(const User &user)
{
    m_currentUser = user;
    accept();
}

void LoginView::logoutCompleted()
{
    m_usernameField->clear();
    m_passwordField->clear();
    clearErrorDisplay();
}
