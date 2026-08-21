// ---------------------------------------------------------------------------
// PublicationController — MVC layer: CONTROLLER (US-02, US-03)
//
// fetchRequested() wird sowohl vom manuellen Klick als auch vom
// ArxivScheduler (taeglich 7 Uhr, US-09) aufgerufen -- fuer den Controller
// macht das keinen Unterschied, die Verdrahtung mit dem Zeitplaner passiert
// in main.cpp (Composition Root).
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QObject>
#include "model/Discipline.h"
#include "model/Publication.h"

class ArxivClient;
class PublicationRepository;
class PublicationViewContract;

class PublicationController : public QObject
{
    Q_OBJECT

public:
    PublicationController(ArxivClient &arxivClient, PublicationRepository &repository,
                                PublicationViewContract &ansicht, QObject *parentObject = nullptr);

    void refreshView();
    void disciplineSelected(Discipline discipline);
    void fetchRequested();
    void publicationsReceived(const QList<Publication> &publications);
    void fetchFailed(const QString &message);

    Discipline selectedDiscipline() const { return m_selectedDiscipline; }

    static const QString kHintNoResults;
    static const QString kMessageFetchSuccessful;

private:
    ArxivClient                 &m_arxivClient;
    PublicationRepository &m_repository;
    PublicationViewContract   &m_view;
    Discipline m_selectedDiscipline = Discipline::Alle;
};
