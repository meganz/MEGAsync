#ifndef QMEGAMESSAGEBOX_H
#define QMEGAMESSAGEBOX_H

#include "qmessagebox.h"

class QMegaMessageBox : QMessageBox
{
public:
    explicit QMegaMessageBox(QWidget *parent = 0) {};

    static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
         const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton);

    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
         const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton);

    static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
         const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton);

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
         const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons = Ok,
                                      QMessageBox::StandardButton defaultButton = NoButton);

private:
    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                    QMessageBox::Icon icon, const QString& title, const QString& text, qreal devPixelRatio,
                     QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);
};

#endif // QMEGAMESSAGEBOX_H
