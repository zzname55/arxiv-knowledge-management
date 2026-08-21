// ---------------------------------------------------------------------------
// Testdoppel fuer die vier Ansichts-Schnittstellen.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>

#include "controller/LoginViewContract.h"
#include "controller/UserManagementViewContract.h"
#include "controller/ReadingListViewContract.h"
#include "controller/PublicationViewContract.h"

class FakeLoginViewContract : public LoginViewContract
{
public:
    QString eingegebenerBenutzername;
    QString eingegebenesPasswort;

    QString  angezeigterFehler;
    int      anzahlFehlerGeloescht = 0;
    bool     anmeldungAbgeschlossenAufgerufen = false;
    User currentUser;
    bool     abmeldungAufgerufen = false;

    QString username() const override { return eingegebenerBenutzername; }
    QString password() const override { return eingegebenesPasswort; }

    void showError(const QString &message) override { angezeigterFehler = message; }
    void clearErrorDisplay() override { angezeigterFehler.clear(); ++anzahlFehlerGeloescht; }
    void loginCompleted(const User &user) override
    {
        anmeldungAbgeschlossenAufgerufen = true;
        currentUser             = user;
    }
    void logoutCompleted() override { abmeldungAufgerufen = true; }
};

class FakePublicationViewContract : public PublicationViewContract
{
public:
    QList<Publication> angezeigteVeroeffentlichungen;
    int     angezeigteTrefferzahl = -1;
    int     gesamtzahl            = -1;
    QString angezeigterHinweis;
    QString angezeigterFehler;
    bool    ladeanzeigeSichtbar = false;
    int     anzahlLadeanzeigeAn = 0;

    void showPublications(const QList<Publication> &publications) override
    {
        angezeigteVeroeffentlichungen = publications;
    }
    void showResultCount(int displayed, int total) override
    {
        angezeigteTrefferzahl = displayed;
        gesamtzahl            = total;
    }
    void showHint(const QString &message) override { angezeigterHinweis = message; }
    void showError(const QString &message) override { angezeigterFehler = message; }
    void showLoadingIndicator(bool visible) override
    {
        ladeanzeigeSichtbar = visible;
        if (visible) {
            ++anzahlLadeanzeigeAn;
        }
    }
};

class FakeReadingListViewContract : public ReadingListViewContract
{
public:
    QList<ReadingListEntry> angezeigteEintraege;
    QString                 angezeigteMeldung;
    QString                 angezeigterFehler;

    void showEntries(const QList<ReadingListEntry> &entries) override { angezeigteEintraege = entries; }
    void showMessage(const QString &message) override { angezeigteMeldung = message; }
    void showError(const QString &message) override { angezeigterFehler = message; }
};

class FakeUserManagementViewContract : public UserManagementViewContract
{
public:
    QList<User> angezeigteBenutzer;
    QString         angezeigteMeldung;
    QString         angezeigterFehler;
    int             anzahlFormularZurueckgesetzt = 0;

    void showUsers(const QList<User> &userList) override { angezeigteBenutzer = userList; }
    void showMessage(const QString &message) override { angezeigteMeldung = message; }
    void showError(const QString &message) override { angezeigterFehler = message; }
    void resetForm() override { ++anzahlFormularZurueckgesetzt; }
};
