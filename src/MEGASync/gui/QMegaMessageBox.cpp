#include "QMegaMessageBox.h"

#include "DialogOpener.h"
#include "MessageDialogComponent.h"
#include "QmlDialogWrapper.h"
#include "Utilities.h"

#include <QPointer>

QString QMegaMessageBox::warningTitle()
{
    return QCoreApplication::translate("MegaApplication", "Warning");
}

QString QMegaMessageBox::errorTitle()
{
    return QCoreApplication::translate("MegaApplication", "Error");
}

QString QMegaMessageBox::fatalErrorTitle()
{
    return QCoreApplication::translate("MegaApplication", "Alert");
}

void QMegaMessageBox::information(const MessageBoxInfo& info)
{
    return show(MessageDialogData::Type::INFORMATION, info);
}

void QMegaMessageBox::warning(const MessageBoxInfo& info)
{
    return show(MessageDialogData::Type::WARNING, info);
}

void QMegaMessageBox::question(const MessageBoxInfo& info)
{
    return show(MessageDialogData::Type::QUESTION, info);
}

void QMegaMessageBox::critical(const MessageBoxInfo& info)
{
    return show(MessageDialogData::Type::CRITICAL, info);
}

void QMegaMessageBox::show(MessageDialogData::Type type, const MessageBoxInfo& info)
{
    auto showDialog = [type, info]()
    {
        QPointer<MessageDialogData> data = new MessageDialogData(type, info, info.parent);
        QPointer<QmlDialogWrapper<MessageDialogComponent>> dialog =
            new QmlDialogWrapper<MessageDialogComponent>(info.parent, data);
        DialogOpener::showMessageBox(dialog, data);
    };

    if (MegaSyncApp->thread() != MegaSyncApp->thread()->currentThread())
    {
        Utilities::queueFunctionInAppThread(
            [showDialog]()
            {
                showDialog();
            });
    }
    else
    {
        showDialog();
    }
}
