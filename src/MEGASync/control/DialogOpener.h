#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include <HighDpiResize.h>

#include <DialogGeometryRetainer.h>

#ifdef _WIN32
#include <ExternalDialogOpener.h>
#endif

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

    protected:
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

        return nullptr;
    }

    template <class DialogType>
    static void removeDialogByClass()
    {
        auto classType = QString::fromUtf8(typeid(DialogType).name());

        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(dialogInfo->getDialogClass() == classType)
            {
                auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);
                if(dialogInfoByType)
                {
                    removeDialog(dialogInfoByType->getDialog());
                    break;
                }
            }
        }
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [func, dialog](){
                func();
                removeDialog(dialog);
            });
            showDialogImpl(dialog);
        }
    }

    template <class DialogType, class CallbackClass>
    static void showDialog(QPointer<DialogType> dialog, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [dialog, caller, func](){
                (caller->*func)(dialog);
                removeDialog(dialog);
            });

            showDialogImpl(dialog);
        }
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [dialog]()
            {
                removeDialog(dialog);
            });

            showDialogImpl(dialog);
        }
    }

    template <class DialogType>
    static void removeDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
     //       dialog->close();
            dialog->deleteLater();
        }
    }

private:
    static QList<std::shared_ptr<DialogInfoBase>> mOpenedDialogs;

    template <class DialogType>
    static void showDialogImpl(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            auto classType = QString::fromUtf8(typeid(DialogType).name());
            auto siblingDialogInfo = findSiblingDialogInfo<DialogType>(classType);

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

#ifdef _WIN32
            ExternalDialogOpener externalOpener;
#endif

#ifdef __APPLE__
            if(dialog->parentWidget() && dialog->windowModality() == Qt::NonModal)
            {
                dialog->setWindowModality(Qt::ApplicationModal);
            }
#else
            if(dialog->windowModality() == Qt::NonModal)
            {
                dialog->setWindowModality(Qt::ApplicationModal);
            }
#endif

            dialog->show();
            dialog->raise();
            dialog->activateWindow();
        }
    }

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

        //This depends on QDialog -> Check if we can templatizate it in order to reuse it with QML
        if(auto widgetdialog = dynamic_cast<QDialog*>(dialog.data()))
        {
           // auto dpiResize = new HighDpiResize(widgetdialog);
        }
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialogInfo(DialogType* dialog)
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType && dialogInfoByType->getDialog() == dialog)
            {
                return dialogInfoByType;
            }
        }

        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findSiblingDialogInfo(const QString& classType)
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType && !dialogInfoByType->getDialogClass().isEmpty())
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
