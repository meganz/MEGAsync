#include "DialogOpener.h"

#include "MessageDialogComponent.h"
#include "MessageDialogData.h"
#include "QmlDialogWrapper.h"

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

void DialogOpener::showMessageDialog(QPointer<QmlMessageDialogWrapper> wrapper,
                                     QPointer<MessageDialogData> msgInfo)
{
    if (wrapper)
    {
        DialogBlocker* blocker(nullptr);

#ifdef Q_OS_MACOS
        // If the message dialog is not WindowModal, the parent is not greyed out.
        // So, we use a dummy dialog WindowModal
        if (msgInfo->parent())
        {
            blocker = new DialogBlocker(wrapper->parentWidget());
            qApp->setActiveWindow(wrapper);
        }
#endif

        wrapper->setWindowModality(Qt::ApplicationModal);
        wrapper->connect(wrapper.data(),
                         &QmlDialogWrapperBase::finished,
                         [msgInfo, wrapper, blocker]()
                         {
                             if (blocker)
                             {
                                 blocker->deleteLater();
                             }

                             if (msgInfo->getFinishFunction())
                             {
                                 msgInfo->getFinishFunction()(msgInfo->result());
                             }

                             removeDialog(wrapper);
                             if (!mDialogsQueue.isEmpty())
                             {
                                 auto queueMsgBox =
                                     std::dynamic_pointer_cast<DialogInfo<QmlMessageDialogWrapper>>(
                                         mDialogsQueue.dequeue());
                                 if (queueMsgBox)
                                 {
                                     auto dialog =
                                         showDialogImpl(queueMsgBox->getDialog(), false, false);
                                     dialog->setIgnoreCloseAllAction(msgInfo->ignoreCloseAll());
                                 }
                             }
                         });

        QString classType = className<QmlMessageDialogWrapper>();
        auto siblingDialogInfo = findSiblingDialogInfo<QmlMessageDialogWrapper>(classType);
        if (siblingDialogInfo && msgInfo->enqueue())
        {
            auto info = std::make_shared<DialogInfo<QmlMessageDialogWrapper>>();
            info->setDialog(wrapper);
            info->setDialogClass(classType);
            mDialogsQueue.enqueue(info);
            info->raise();
        }
        else
        {
            auto dialog = showDialogImpl(wrapper, false, false);
            dialog->setIgnoreCloseAllAction(msgInfo->ignoreCloseAll());
        }
        if (msgInfo->getParentWidget())
        {
            QPoint parentCenter = msgInfo->getParentWidget()->geometry().center();
            QSize dialogSize = wrapper->size();
            wrapper->move(QPoint((parentCenter.x() - dialogSize.width() / 2),
                                 (parentCenter.y() - dialogSize.height() / 2)));
        }
    }
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
