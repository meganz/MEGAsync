#ifndef MESSAGE_DIALOG_OPENER_H
#define MESSAGE_DIALOG_OPENER_H

#include "MessageDialogData.h"

class MessageDialogOpener
{
public:
    explicit MessageDialogOpener() = delete;
    virtual ~MessageDialogOpener() = delete;

    static void success(const MessageDialogInfo& info);
    static void information(const MessageDialogInfo& info);
    static void warning(const MessageDialogInfo& info);
    static void question(const MessageDialogInfo& info);
    static void critical(const MessageDialogInfo& info);

private:
    static void show(MessageDialogData::Type type, const MessageDialogInfo& info);
};

#endif // MESSAGE_DIALOG_OPENER_H
