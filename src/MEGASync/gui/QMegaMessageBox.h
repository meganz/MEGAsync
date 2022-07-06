#ifndef QMEGAMESSAGEBOX_H
#define QMEGAMESSAGEBOX_H

#include "qmessagebox.h"

class QMegaMessageBox : QMessageBox
{
public:
    explicit QMegaMessageBox(QWidget *parent = 0) : QMessageBox(parent) {};

    static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText);

    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText);

    static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText);

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText);

private:
    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                    QMessageBox::Icon icon, const QString& title, const QString& text,
                     QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton, Qt::TextFormat format);
};

#endif // QMEGAMESSAGEBOX_H
