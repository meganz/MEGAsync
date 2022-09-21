#ifndef QMEGAMESSAGEBOX_H
#define QMEGAMESSAGEBOX_H

#include "qmessagebox.h"
#include <QMap>

class QMegaMessageBox : QMessageBox
{
public:
    explicit QMegaMessageBox(QWidget *parent = 0) : QMessageBox(parent) {};

    static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, QMap<QMessageBox::StandardButton, QString> textByButton = QMap<QMessageBox::StandardButton, QString>(), Qt::TextFormat format = Qt::TextFormat::PlainText);

    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText, QMap<QMessageBox::StandardButton, QString> textByButton = QMap<QMessageBox::StandardButton, QString>());

    static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText, QMap<QMessageBox::StandardButton, QString> textByButton = QMap<QMessageBox::StandardButton, QString>());

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton, Qt::TextFormat format = Qt::TextFormat::PlainText, QMap<QMessageBox::StandardButton, QString> textByButton = QMap<QMessageBox::StandardButton, QString>());

private:
    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                    QMessageBox::Icon icon, const QString& title, const QString& text,
                     QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton, Qt::TextFormat format, QMap<QMessageBox::StandardButton, QString> textByButton = QMap<QMessageBox::StandardButton, QString>());
};

#endif // QMEGAMESSAGEBOX_H
