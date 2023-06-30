#include "QMegaMessageBox.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

#include "DialogOpener.h"

QString QMegaMessageBox::warningTitle()
{
    return QCoreApplication::translate("MegaApplication", "Warning");
}

QString QMegaMessageBox::errorTitle()
{
    return QCoreApplication::translate("MegaApplication", "Error");
}

void QMegaMessageBox::information(const MessageBoxInfo& info)
{
    return showNewMessageBox(Information, info);
}

void QMegaMessageBox::warning(const MessageBoxInfo& info)
{
    return showNewMessageBox(Warning, info);
}

void QMegaMessageBox::question(const MessageBoxInfo& info)
{
    return showNewMessageBox(Question, info);
}

void QMegaMessageBox::critical(const MessageBoxInfo& info)
{
    return showNewMessageBox(Critical, info);
}

bool QMegaMessageBox::event(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        event->ignore();
        return true;
    }

    return QMessageBox::event(event);
}

void QMegaMessageBox::showNewMessageBox(Icon icon, const MessageBoxInfo& info)
{
    QMessageBox* msgBox = new QMegaMessageBox(info.parent);

    msgBox->setIcon(icon);
    msgBox->setWindowTitle(info.title);
    msgBox->setText(info.text);
    msgBox->setInformativeText(info.informativeText);
    msgBox->setTextFormat(info.textFormat);
    msgBox->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);

    QDialogButtonBox *buttonBox = msgBox->findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    uint mask = FirstButton;
    while (mask <= LastButton) {
     uint sb = info.buttons & mask;
     mask <<= 1;
     if (!sb)
         continue;
     StandardButton buttonType = static_cast<StandardButton>(sb);
     QPushButton *button = msgBox->addButton(buttonType);

     //Change button text if needed
     if(info.buttonsText.contains(buttonType))
     {
         button->setText(info.buttonsText.value((StandardButton)sb));
     }

     // Choose the first accept role as the default
     if (msgBox->defaultButton())
         continue;
     if ((info.defaultButton == NoButton && buttonBox && buttonBox->buttonRole((QAbstractButton*)button) == QDialogButtonBox::AcceptRole)
             || (info.defaultButton != NoButton && sb == uint(info.defaultButton)))
     {
         msgBox->setDefaultButton(button);
     }
    }

    if(!info.iconPixmap.isNull())
    {
        msgBox->setIconPixmap(info.iconPixmap);
    }

    DialogOpener::showMessageBox(msgBox, info.finishFunc, info.enqueue);
}
