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
    class DialogInfoBase
    {
    public:
        DialogInfoBase() = default;
        virtual ~DialogInfoBase() = default;

        QString getDialogClass() const {return mDialogClass;}
        void setDialogClass(const QString &newDialogClass) {mDialogClass = newDialogClass;}

    private:
        QString mDialogClass;
    };

    template <class DialogType>
    class DialogInfo : public DialogInfoBase
    {
    public:
        QPointer<DialogType> getDialog() const { return mDialog;}
        void setDialog(QPointer<DialogType> newDialog)
        {
            mDialog = newDialog;
        }

        void clear()
        {
            mDialogClass.clear();
            DialogOpener::removeDialog<DialogType>(mDialog);
        }

        bool isEmpty()
        {
            return mDialog == nullptr;
        }

        bool operator==(const DialogInfo &info)
        {
            return info.mDialog == mDialog ? true : false;
        }


    private:
        QPointer<DialogType> mDialog;
    };


public:
    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->getDialogClass() == classType)
            {
                return std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);
            }
        }

        return std::make_shared<DialogOpener::DialogInfo<DialogType>>();
    }

    template <class DialogType>
    static bool removeDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->getDialogClass() == classType)
            {
                auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);
                return removeDialog(dialogInfoByType->getDialog());
            }
        }

        return false;
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [func, dialog](){
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
            dialog->connect(dialog.data(), &DialogType::finished, [dialog, caller, func](){
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
            auto siblingDialogInfo = findSiblingDialogInfo<DialogType>(dialog.data(),classType);

            if(siblingDialogInfo)
            {
                if(siblingDialogInfo->getDialog() != dialog)
                {
                    QRect siblingGeometry = siblingDialogInfo->getDialog()->geometry();
                    bool siblingIsMaximized = siblingDialogInfo->getDialog()->isMaximized();

                    removeDialog(siblingDialogInfo->getDialog());
                    siblingDialogInfo->setDialog(dialog);
                    siblingDialogInfo->setDialogClass(classType);

                    initDialog(dialog);

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
                std::shared_ptr<DialogInfo<DialogType>> info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);

                initDialog(dialog);
            }

            ExternalDialogOpener externalOpener;

            if(dialog->parent())
            {
                dialog->setWindowModality(Qt::WindowModal);
            }
            else
            {
#ifndef __APPLE__
                dialog->setModal(true);
#else
                dialog->setModal(false);
#endif
            }

            dialog->show();
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
    static void initDialog(QPointer<DialogType> dialog)
    {
        dialog->connect(dialog, &QObject::destroyed, [dialog](){
            auto info = findDialogInfo<DialogType>(dialog);
            if(info)
            {
                mOpenedDialogs.removeOne(info);
            }
        });

        auto dpiResize = new HighDpiResize(dialog);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    static QList<std::shared_ptr<DialogInfoBase>> mOpenedDialogs;

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialogInfo(QDialog* dialog)
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType->getDialog() == dialog)
            {
                return dialogInfoByType;
            }
        }

        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findSiblingDialogInfo(QDialog*, const QString& classType)
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType->getDialog() && !dialogInfoByType->getDialogClass().isEmpty())
            {
                if(/*dialogInfo->getDialog() != dialog
                        && dialogInfo->dialog->parent() == dialog->parent()
                        && */dialogInfoByType->getDialogClass()  == classType)
                {
                    return dialogInfoByType;
                }
            }
        }

        return nullptr;
    }
};


#endif // DIALOGOPENER_H
