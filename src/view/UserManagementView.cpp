#include "view/UserManagementView.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "model/UserManagementService.h"
#include "view/SortableTableItem.h"

UserManagementView::UserManagementView(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    buildUi();
    selectionChanged();
}

void UserManagementView::buildUi()
{
    auto *heading = new QLabel(tr("User Management"), this);
    QFont ueberschriftSchrift = heading->font();
    ueberschriftSchrift.setPointSize(ueberschriftSchrift.pointSize() + 2);
    ueberschriftSchrift.setBold(true);
    heading->setFont(ueberschriftSchrift);

    m_table = new QTableWidget(0, SpaltenAnzahl, this);
    m_table->setObjectName(QStringLiteral("benutzerTabelle"));
    m_table->setHorizontalHeaderLabels({ tr("Username"), tr("DisplayName"), tr("Role"), tr("Status") });
    m_table->horizontalHeader()->setSectionResizeMode(SpalteAnzeigename, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->horizontalHeader()->setSortIndicatorShown(true);
    m_table->sortByColumn(SpalteBenutzername, Qt::AscendingOrder);

    m_usernameField = new QLineEdit(this);
    m_usernameField->setObjectName(QStringLiteral("neuBenutzernameFeld"));
    m_usernameField->setPlaceholderText(tr("e.g. ma03"));

    m_displayNameField = new QLineEdit(this);
    m_displayNameField->setObjectName(QStringLiteral("neuAnzeigenameFeld"));
    m_displayNameField->setPlaceholderText(tr("e.g. Mia Meier"));

    m_passwordField = new QLineEdit(this);
    m_passwordField->setObjectName(QStringLiteral("neuPasswortFeld"));
    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setPlaceholderText(tr("at least %1 characters").arg(UserManagementService::kMinimumPasswordLength));

    m_roleSelector = new QComboBox(this);
    m_roleSelector->setObjectName(QStringLiteral("rollenAuswahl"));
    for (const UserRole role : allRoles()) {
        m_roleSelector->addItem(roleToText(role), QVariant::fromValue(static_cast<int>(role)));
    }

    auto *formular = new QFormLayout;
    formular->addRow(tr("Username *"), m_usernameField);
    formular->addRow(tr("DisplayName *"),  m_displayNameField);
    formular->addRow(tr("Password *"),     m_passwordField);
    formular->addRow(tr("Role *"),        m_roleSelector);

    auto *formularKasten = new QGroupBox(tr("Create New User"), this);
    formularKasten->setLayout(formular);

    m_createButton      = new QPushButton(tr("Create"), this);
    m_createButton->setObjectName(QStringLiteral("anlegenKnopf"));
    m_changeRoleButton = new QPushButton(tr("Change Role"), this);
    m_changeRoleButton->setObjectName(QStringLiteral("rolleAendernKnopf"));
    m_deactivateButton = new QPushButton(tr("Deactivate"), this);
    m_deactivateButton->setObjectName(QStringLiteral("deaktivierenKnopf"));
    m_activateButton   = new QPushButton(tr("Activate"), this);
    m_activateButton->setObjectName(QStringLiteral("aktivierenKnopf"));

    auto *knopfleiste = new QHBoxLayout;
    knopfleiste->addWidget(m_createButton);
    knopfleiste->addWidget(m_changeRoleButton);
    knopfleiste->addWidget(m_deactivateButton);
    knopfleiste->addWidget(m_activateButton);
    knopfleiste->addStretch();

    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName(QStringLiteral("meldungsAnzeige"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setMinimumHeight(28);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(heading);
    mainLayout->addWidget(m_table, 1);
    mainLayout->addWidget(formularKasten);
    mainLayout->addLayout(knopfleiste);
    mainLayout->addWidget(m_messageLabel);

    auto *pflichtfeldHinweis = new QLabel(tr("* Required field"), this);
    pflichtfeldHinweis->setStyleSheet(QStringLiteral("color: #777777;"));
    mainLayout->addWidget(pflichtfeldHinweis);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &UserManagementView::selectionChanged);

    connect(m_createButton, &QPushButton::clicked, this, [this]() {
        emit createRequested(m_usernameField->text().trimmed(), m_displayNameField->text().trimmed(),
                               m_passwordField->text(), selectedRole());
    });
    connect(m_changeRoleButton, &QPushButton::clicked, this, [this]() {
        if (const User *user = selectedUser()) {
            emit changeRoleRequested(user->id(), selectedRole());
        }
    });
    connect(m_deactivateButton, &QPushButton::clicked, this, [this]() {
        if (const User *user = selectedUser()) {
            emit deactivateRequested(user->id());
        }
    });
    connect(m_activateButton, &QPushButton::clicked, this, [this]() {
        if (const User *user = selectedUser()) {
            emit activateRequested(user->id());
        }
    });
}

UserRole UserManagementView::selectedRole() const
{
    return static_cast<UserRole>(m_roleSelector->currentData().toInt());
}

const User *UserManagementView::selectedUser() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return nullptr;
    }
    const QTableWidgetItem *ersteZelle = m_table->item(row, SpalteBenutzername);
    if (ersteZelle == nullptr) {
        return nullptr;
    }
    const int gesuchteId = ersteZelle->data(kDatensatzIdRolle).toInt();
    for (const User &user : m_displayedUsers) {
        if (user.id() == gesuchteId) {
            return &user;
        }
    }
    return nullptr;
}

void UserManagementView::showUsers(const QList<User> &userList)
{
    m_displayedUsers = userList;

    const bool sortierungWarAn = m_table->isSortingEnabled();
    m_table->setSortingEnabled(false);

    m_table->setRowCount(static_cast<int>(userList.size()));

    for (int row = 0; row < userList.size(); ++row) {
        const User &user = userList.at(row);

        auto *benutzernameZelle = new SortableTableItem(user.username(), user.username());
        benutzernameZelle->setData(kDatensatzIdRolle, user.id());
        m_table->setItem(row, SpalteBenutzername, benutzernameZelle);

        m_table->setItem(row, SpalteAnzeigename,
                           new SortableTableItem(user.displayName(), user.displayName()));

        m_table->setItem(row, SpalteRolle,
                           new SortableTableItem(roleToText(user.role()),
                                                           static_cast<int>(allRoles().indexOf(user.role()))));

        m_table->setItem(row, SpalteStatus,
                           new SortableTableItem(user.isActive() ? tr("Active") : tr("Locked"),
                                                           user.isActive() ? 1 : 0));
    }

    m_table->setSortingEnabled(sortierungWarAn);
    selectionChanged();
}

void UserManagementView::selectionChanged()
{
    const User *user = selectedUser();
    const bool      hatAuswahl = (user != nullptr);

    m_changeRoleButton->setEnabled(hatAuswahl);
    m_deactivateButton->setEnabled(hatAuswahl && user->isActive());
    m_activateButton->setEnabled(hatAuswahl && !user->isActive());
}

void UserManagementView::showMessage(const QString &message)
{
    m_messageLabel->setStyleSheet(QStringLiteral("color: #1b5e20;"));
    m_messageLabel->setText(message);
}

void UserManagementView::showError(const QString &message)
{
    m_messageLabel->setStyleSheet(QStringLiteral("color: #b00020;"));
    m_messageLabel->setText(message);
}

void UserManagementView::resetForm()
{
    m_usernameField->clear();
    m_displayNameField->clear();
    m_passwordField->clear();
    m_roleSelector->setCurrentIndex(0);
    m_usernameField->setFocus();
}
