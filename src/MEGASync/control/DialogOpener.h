#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include <HighDpiResize.h>

#include <QDialog>
#include <QPointer>
#include <functional>
#include <memory>

class DialogOpener
{
private:
    struct DialogInfo
    {
        QString dialogClass;
        QPointer<QDialog> dialog;

        template <class DialogType>
        DialogType* getDialog()
        {
            return qobject_cast<DialogType*>(dialog);
        }

        void clear()
        {
            dialogClass.clear();
            removeDialog(dialog);
        }

        bool isEmpty()
        {
            return dialog == nullptr;
        }

        bool operator==(const DialogInfo &info);

    };

public:
    template <class DialogType>
    static std::shared_ptr<DialogInfo> findDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->dialogClass == classType)
            {
                return dialogInfo;
            }
        }

        return std::make_shared<DialogOpener::DialogInfo>();
    }

    template <class DialogType>
    static bool removeDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->dialogClass == classType)
            {
                return removeDialog(dialogInfo->dialog);
            }
        }

        return false;
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &QDialog::finished, [func, dialog](){
                func();
            });
            showDialog(dialog);
        }
    }

    template <class DialogType, class CallbackClass>
    static void showDialog(QPointer<DialogType> dialog, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &QDialog::finished, [dialog, caller, func](){
                (caller->*func)(dialog);
            });

            showDialog(dialog);
        }
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            auto classType = QString::fromUtf8(typeid(DialogType).name());
            auto siblingDialog = findSiblingDialogInfo(dialog.data(),classType);

            if(siblingDialog)
            {
                removeDialog(siblingDialog->dialog);
                siblingDialog->dialog = dialog;
                siblingDialog->dialogClass = classType;
            }
            else
            {
                std::shared_ptr<DialogInfo> info = std::make_shared<DialogInfo>();
                info->dialog = dialog;
                info->dialogClass = classType;
                mOpenedDialogs.append(info);
            }

            dialog->connect(dialog, &QObject::destroyed, [dialog](){
                auto info = findDialogInfo(dialog);
                if(info)
                {
                    mOpenedDialogs.removeOne(info);
                }
            });

            if(!dialog->isVisible())
            {
                auto dpiResize = new HighDpiResize(dialog);
            }

            dialog->setAttribute(Qt::WA_DeleteOnClose);

            auto dialogModality = dialog->windowModality();
            if(dialog->parent() && dialogModality != Qt::WindowModal)
            {
                dialog->setWindowModality(Qt::WindowModal);
                dialog->show();
            }
//#ifdef __APPLE__
//            else if(dialogModality == Qt::ApplicationModal && !dialog->parent())
//            {
//                dialog->exec();
//            }
//#endif
            else
            {
                dialogModality == Qt::NonModal ? dialog->show() : dialog->open();
            }

            dialog->raise();
            dialog->activateWindow();
        }
    }

    template <class DialogType>
    static bool removeDialog(QPointer<DialogType>& dialog)
    {
        if(dialog)
        {
            dialog->close();
            if(!dialog->testAttribute(Qt::WA_DeleteOnClose))
            {
                dialog->deleteLater();
                dialog = nullptr;
                return true;
            }
        }

        return false;
    }

private:
    static QList<std::shared_ptr<DialogInfo>> mOpenedDialogs;

    static std::shared_ptr<DialogInfo> findDialogInfo(QDialog* dialog);
    static std::shared_ptr<DialogInfo> findSiblingDialogInfo(QDialog* dialog, const QString& classType);
};


#endif // DIALOGOPENER_H
