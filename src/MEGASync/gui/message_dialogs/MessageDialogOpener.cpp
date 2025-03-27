#include "MessageDialogOpener.h"

#include "DialogOpener.h"
#include "MessageDialogComponent.h"
#include "QmlDialogWrapper.h"
#include "Utilities.h"

#include <QPointer>

void MessageDialogOpener::information(const MessageDialogInfo& info)
{
    return show(MessageDialogData::Type::INFORMATION, info);
}

void MessageDialogOpener::warning(const MessageDialogInfo& info)
{
    return show(MessageDialogData::Type::WARNING, info);
}

void MessageDialogOpener::question(const MessageDialogInfo& info)
{
    return show(MessageDialogData::Type::QUESTION, info);
}

void MessageDialogOpener::critical(const MessageDialogInfo& info)
{
    return show(MessageDialogData::Type::CRITICAL, info);
}

void MessageDialogOpener::show(MessageDialogData::Type type, const MessageDialogInfo& info)
{
    auto showDialog = [type, info]()
    {
        QPointer<MessageDialogData> data = new MessageDialogData(type, info, info.parent);
        QPointer<QmlDialogWrapper<MessageDialogComponent>> dialog =
            new QmlDialogWrapper<MessageDialogComponent>(info.parent, data);
        DialogOpener::showMessageDialog(dialog, data);
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
