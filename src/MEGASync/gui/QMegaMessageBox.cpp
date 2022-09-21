#include "QMegaMessageBox.h"
#include "HighDpiResize.h"
#include <QDialogButtonBox>

QMessageBox::StandardButton QMegaMessageBox::information(QWidget *parent, const QString &title,
    const QString &text, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton, QMap<StandardButton, QString> textByButton)
{
    return showNewMessageBox(parent, Information, title, text, buttons, defaultButton, textByButton);
}

QMessageBox::StandardButton QMegaMessageBox::warning(QWidget *parent, const QString &title,
    const QString &text, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton, QMap<StandardButton, QString> textByButton)
{
    return showNewMessageBox(parent, Warning, title, text, buttons, defaultButton, textByButton);
}

QMessageBox::StandardButton QMegaMessageBox::question(QWidget *parent, const QString &title,
    const QString &text, QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton, QMap<StandardButton, QString> textByButton)
{
    return showNewMessageBox(parent, Question, title, text, buttons, defaultButton, textByButton);
}

QMessageBox::StandardButton QMegaMessageBox::critical(QWidget *parent, const QString &title,
    const QString &text,  QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton, QMap<StandardButton, QString> textByButton)
{
    return showNewMessageBox(parent, Critical, title, text, buttons, defaultButton, textByButton);
}

QMessageBox::StandardButton QMegaMessageBox::showNewMessageBox(QWidget *parent, QMessageBox::Icon icon,
    const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton, QMap<QMessageBox::StandardButton, QString> textByButton)
{
    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
    msgBox.setTextFormat(Qt::TextFormat::PlainText);
    HighDpiResize hDpiResizer(&msgBox);

    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
     uint sb = buttons & mask;
     mask <<= 1;
     if (!sb)
         continue;
     QMessageBox::StandardButton buttonType = static_cast<QMessageBox::StandardButton>(sb);
     QPushButton *button = msgBox.addButton(buttonType);

     //Change button text if needed
     if(textByButton.contains(buttonType))
     {
         msgBox.setButtonText(buttonType, textByButton.value((QMessageBox::StandardButton)sb));
     }

     // Choose the first accept role as the default
     if (msgBox.defaultButton())
         continue;
     if ((defaultButton == QMessageBox::NoButton && buttonBox && buttonBox->buttonRole((QAbstractButton*)button) == QDialogButtonBox::AcceptRole)
             || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
     {
         msgBox.setDefaultButton(button);
     }
    }
    if (msgBox.exec() == -1)
    return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}
