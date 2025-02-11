#ifndef QMEGAMESSAGEBOX_H
#define QMEGAMESSAGEBOX_H

#include "MessageDialogData.h"

class QMegaMessageBox
{
public:
    explicit QMegaMessageBox() = delete;
    virtual ~QMegaMessageBox() = delete;

    static QString warningTitle();
    static QString errorTitle();
    static QString fatalErrorTitle();

    static void information(const MessageBoxInfo& info);
    static void warning(const MessageBoxInfo& info);
    static void question(const MessageBoxInfo& info);
    static void critical(const MessageBoxInfo& info);

private:
    static void show(MessageDialogData::Type type, const MessageBoxInfo& info);
};

#endif // QMEGAMESSAGEBOX_H
