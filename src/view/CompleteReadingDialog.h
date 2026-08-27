// ---------------------------------------------------------------------------
// CompleteReadingDialog — MVC layer: VIEW (W5, AK-05.3/05.4)
// ---------------------------------------------------------------------------
#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QPlainTextEdit;
class QSpinBox;

class CompleteReadingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompleteReadingDialog(const QString &title, QWidget *parentWidget = nullptr);

    int rating() const;
    QString note() const;
    void showError(const QString &message);

private:
    void buildUi(const QString &title);
    void validateInputAndAccept();

    QSpinBox       *m_ratingField = nullptr;
    QPlainTextEdit *m_noteField     = nullptr;
    QLabel         *m_errorLabel = nullptr;
};
