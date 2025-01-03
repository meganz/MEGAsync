#include "QMegaMessageBox.h"

#include "DialogOpener.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>

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
    auto showMsgBox = [icon, info]()
    {
        QMessageBox* msgBox = new QMegaMessageBox(info.parent);

        msgBox->setIcon(icon);
        msgBox->setWindowTitle(info.title.isEmpty() ? QString::fromLatin1("MEGA") : info.title);
        msgBox->setText(info.text);
        msgBox->setInformativeText(info.informativeText);
        msgBox->setTextFormat(info.textFormat);
        msgBox->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);
        if(info.checkBox)
        {
#ifdef Q_OS_MACOS
            //This little hack is done in order to have the same font as the Informative text on macOS (the only one with different font)
            info.checkBox->setFont(qApp->font("QTipLabel"));
#endif
            msgBox->setCheckBox(info.checkBox);
        }
        QDialogButtonBox* buttonBox = msgBox->findChild<QDialogButtonBox*>();
        Q_ASSERT(buttonBox != 0);

#ifdef Q_OS_LINUX
        buttonBox->setStyleSheet(QLatin1String("background: rgb(246, 245, 244);"));
#endif

        uint mask = FirstButton;
        while(mask <= LastButton)
        {
            int sb = info.buttons & mask;
            mask <<= 1;
            if(!sb)
                continue;
            StandardButton buttonType = static_cast<StandardButton>(sb);
            QPushButton* button = msgBox->addButton(buttonType);
#ifdef Q_OS_MACOS
            // Work-around for default buttons not highlighted correctly in MacOS(
            button->setFixedHeight(32);
#endif \
    //Change button text if needed
            if(info.buttonsText.contains(buttonType))
            {
                button->setText(info.buttonsText.value((StandardButton) sb));
            }

            // Choose the first accept role as the default
            if(msgBox->defaultButton())
                continue;
            if((info.defaultButton == NoButton && buttonBox &&
                   buttonBox->buttonRole((QAbstractButton*) button) ==
                       QDialogButtonBox::AcceptRole) ||
                (info.defaultButton != NoButton && sb == static_cast<int>((info.defaultButton))))
            {
                msgBox->setDefaultButton(button);
            }
        }

        if(!info.iconPixmap.isNull())
        {
            msgBox->setIconPixmap(info.iconPixmap);
        }

        if(!info.checkboxText.isEmpty())
        {
            QCheckBox* checkbox = new QCheckBox(info.checkboxText);
            msgBox->setCheckBox(checkbox);
        }

        // Prevent showing context menu in texts
        for (auto childLabel: msgBox->findChildren<QLabel*>())
        {
            childLabel->setContextMenuPolicy(Qt::PreventContextMenu);
        }

        DialogOpener::showMessageBox(msgBox, info);
    };

    //ALWAYS show messagebox on GUI thread
    if(MegaSyncApp->thread() != MegaSyncApp->thread()->currentThread())
    {
        Utilities::queueFunctionInAppThread([showMsgBox]() { showMsgBox(); });
    }
    else
    {
        showMsgBox();
    }
}
