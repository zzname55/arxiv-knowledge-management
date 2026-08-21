// ---------------------------------------------------------------------------
// ArxivAtomParser — MVC layer: MODEL
// Wandelt die Atom-Antwort der arXiv-API in Publication-Objekte um.
// Vom Netzzugriff getrennt, um gegen fest hinterlegte Antworten testbar zu sein.
// ---------------------------------------------------------------------------
#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include "model/Publication.h"

class ArxivAtomParser
{
public:
    /// @param failure optionaler Ausgabeparameter mit einer deutschen errorMessage.
    /// entries ohne arXiv-ID werden uebergangen (kein Duplikatschutz moeglich).
    /// Ein Feed ohne Treffer ist kein Fehler.
    static QList<Publication> parse(const QByteArray &atomAntwort, QString *failure = nullptr);

private:
    ArxivAtomParser() = delete;
};
