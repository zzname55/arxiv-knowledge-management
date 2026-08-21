// ---------------------------------------------------------------------------
// OperationResult — MVC layer: MODEL
// Einheitliche Rueckgabe aller aendernden Vorgaenge. Das Model formuliert die
// message, der Controller reicht sie nur weiter.
// ---------------------------------------------------------------------------
#pragma once

#include <QString>

struct OperationResult {
    bool    successful = false;
    QString errorMessage;

    static OperationResult success() { return { true, QString() }; }
    static OperationResult failure(const QString &message) { return { false, message }; }
};
