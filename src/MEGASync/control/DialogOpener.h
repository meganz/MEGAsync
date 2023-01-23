#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include <HighDpiResize.h>
#include <ExternalDialogOpener.h>
#include <DialogGeometryRetainer.h>

#include <QDialog>
#include <QPointer>
#include <functional>
#include <memory>

class DialogOpener
{

private:
    class DialogInfo
    {
    public:
        QPointer<QDialog> getDialog() const { return mDialog;}
        QString getDialogClass() const {return mDialogClass;}
        void setDialogClass(const QString &newDialogClass) {mDialogClass = newDialogClass;}

        template <class DialogType>
        void setDialog(QPointer<DialogType> newDialog)
        {
            mDialog = newDialog;
        }

        template <class DialogType>
        DialogType* getDialogByClass()
        {
            return qobject_cast<DialogType*>(mDialog);
        }

        void clear()
        {
            mDialogClass.clear();
            DialogOpener::removeDialog(mDialog);
        }

        bool isEmpty()
        {
            return mDialog == nullptr;
        }

        bool operator==(const DialogInfo &info);

    private:
        QString mDialogClass;
        QPointer<QDialog> mDialog;
    };

public:
    template <class DialogType>
    static std::shared_ptr<DialogInfo> findDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->getDialogClass() == classType)
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
            if(dialogInfo->getDialogClass() == classType)
            {
                return removeDialog(dialogInfo->getDialog());
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
                if(siblingDialog->getDialog() != dialog)
                {
                    QRect siblingGeometry = siblingDialog->getDialog()->geometry();
                    bool siblingIsMaximized = siblingDialog->getDialog()->isMaximized();

                    removeDialog(siblingDialog->getDialog());
                    siblingDialog->setDialog(dialog);
                    siblingDialog->setDialogClass(classType);

                    initDialog(dialog, siblingDialog);

                    if(siblingIsMaximized)
                    {
                        dialog->setWindowState(Qt::WindowMaximized);
                    }
                    else if(siblingGeometry.isValid())
                    {
                        dialog->setGeometry(siblingGeometry);
                    }
                }
            }
            else
            {
                std::shared_ptr<DialogInfo> info = std::make_shared<DialogInfo>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);

                initDialog(dialog, info);
            }

            ExternalDialogOpener externalOpener;

            if(dialog->parent())
            {
                dialog->open();
            }
            else
            {
#ifndef __APPLE__
                dialog->setModal(true);
#else
                dialog->setModal(false);
#endif
                dialog->show();
            }

            dialog->raise();
            dialog->activateWindow();
        }
    }

    template <class DialogType>
    static bool removeDialog(QPointer<DialogType> dialog)
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
    template <class DialogType>
    static void initDialog(QPointer<DialogType> dialog, std::shared_ptr<DialogInfo> info)
    {
        dialog->connect(dialog, &QObject::destroyed, [dialog](){
            auto info = findDialogInfo(dialog);
            if(info)
            {
                mOpenedDialogs.removeOne(info);
            }
        });

        auto dpiResize = new HighDpiResize(dialog);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    static QList<std::shared_ptr<DialogInfo>> mOpenedDialogs;

    static std::shared_ptr<DialogInfo> findDialogInfo(QDialog* dialog);
    static std::shared_ptr<DialogInfo> findSiblingDialogInfo(QDialog* dialog, const QString& classType);
};


#endif // DIALOGOPENER_H
