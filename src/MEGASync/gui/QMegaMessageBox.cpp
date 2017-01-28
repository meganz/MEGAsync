#include "QMegaMessageBox.h"
#include <QDialogButtonBox>

QMessageBox::StandardButton QMegaMessageBox::information(QWidget *parent, const QString &title,
    const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, Information, title, text, devPixelRatio, buttons, defaultButton);
}

QMessageBox::StandardButton QMegaMessageBox::warning(QWidget *parent, const QString &title,
    const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, Warning, title, text, devPixelRatio, buttons, defaultButton);
}

QMessageBox::StandardButton QMegaMessageBox::question(QWidget *parent, const QString &title,
    const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, Question, title, text, devPixelRatio, buttons, defaultButton);
}

QMessageBox::StandardButton QMegaMessageBox::critical(QWidget *parent, const QString &title,
    const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, Critical, title, text, devPixelRatio, buttons, defaultButton);
}

QMessageBox::StandardButton QMegaMessageBox::showNewMessageBox(QWidget *parent, QMessageBox::Icon icon,
    const QString &title, const QString &text, qreal devPixelRatio, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
    switch (icon)
    {
        case Information:
        case Warning:
        case Question:
        case Critical:
            msgBox.setIconPixmap(QPixmap(devPixelRatio < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                       : QString::fromUtf8(":/images/mbox-warning@2x.png")));
        default:
            break;
    }

    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
     uint sb = buttons & mask;
     mask <<= 1;
     if (!sb)
         continue;
     QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
     // Choose the first accept role as the default
     if (msgBox.defaultButton())
         continue;
     if ((defaultButton == QMessageBox::NoButton && buttonBox && buttonBox->buttonRole((QAbstractButton*)button) == QDialogButtonBox::AcceptRole)
         || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
         msgBox.setDefaultButton(button);
    }
    if (msgBox.exec() == -1)
    return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}
