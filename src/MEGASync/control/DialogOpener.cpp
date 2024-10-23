#include "DialogOpener.h"

#include <QApplication>
#include <QOperatingSystemVersion>

QList<std::shared_ptr<DialogOpener::DialogInfoBase>> DialogOpener::mOpenedDialogs = QList<std::shared_ptr<DialogOpener::DialogInfoBase>>();
QQueue<std::shared_ptr<DialogOpener::DialogInfoBase>> DialogOpener::mDialogsQueue = QQueue<std::shared_ptr<DialogOpener::DialogInfoBase>>();
QMap<QString, DialogOpener::GeometryInfo> DialogOpener::mSavedGeometries = QMap<QString, DialogOpener::GeometryInfo>();


#ifdef Q_OS_WINDOWS
ExternalDialogOpener::ExternalDialogOpener()
    : QWidget(nullptr, Qt::SubWindow)
{
    if(QOperatingSystemVersion::current() <= QOperatingSystemVersion::Windows10)
    {
        setAttribute(Qt::WA_DeleteOnClose, true);
        setWindowFlag(Qt::WindowStaysOnBottomHint, true);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setFixedSize(0,0);
        show();
        raise();
        activateWindow();
    }
}

ExternalDialogOpener::~ExternalDialogOpener()
{
    close();
}
#endif

DialogBlocker::DialogBlocker(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setGeometry(QRect(1,1,1,1));
    open();
}

DialogBlocker::~DialogBlocker()
{
    close();
}

QList<QPointer<QWidget>> DialogOpener::getAllOpenedDialogs()
{
    QList<QPointer<QWidget>> dialogs;

    foreach (const auto& dialogInfo, mOpenedDialogs)
    {
        auto dialogPtr = std::static_pointer_cast<DialogInfo<QWidget>>(dialogInfo);
        if (dialogPtr)
        {
            auto dialog = dialogPtr->getDialog();
            if (dialog)
            {
                dialogs.append(dialog);
            }
        }
    }

    return dialogs;
}
