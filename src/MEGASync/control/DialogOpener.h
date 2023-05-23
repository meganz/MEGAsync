#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include <HighDpiResize.h>
#include <Platform.h>

#include <QDialog>
#include <QPointer>
#include <QMap>

#include <functional>
#include <memory>
#include <type_traits>

#include <QMap>
#include <QApplication>

#ifdef Q_OS_WINDOWS
class ExternalDialogOpener : public QWidget
{
public:
    ExternalDialogOpener();
    ~ExternalDialogOpener();
};
#endif

class DialogBlocker : public QDialog
{
public:
    DialogBlocker(QWidget* parent);
    ~DialogBlocker();
};

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

        virtual void raise(bool raiseIfMinimized = false) = 0;
        virtual void close() = 0;
        virtual bool isParent(QObject* parent) = 0;

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

        void raise(bool raiseIfMinimized = false) override
        {
            if(raiseIfMinimized && mDialog->isMinimized())
            {
                mDialog->showNormal();
            }

            if(!mDialog->isMinimized())
            {
                mDialog->raise();
                mDialog->activateWindow();
            }
        }

        void close() override
        {
            Platform::getInstance()->closeFileFolderSelectors(mDialog);
            mDialog->close();
        }

        void clear()
        {
            mDialogClass.clear();
            DialogOpener::removeDialog<DialogType>(mDialog);
        }

        bool isParent(QObject* parent) override
        {
            return mDialog->parent() == parent;
        }

        bool operator==(const DialogInfo &info)
        {
            return (info.mDialog == mDialog);
        }


    private:
        QPointer<DialogType> mDialog;
    };

    struct GeometryInfo
    {
        bool maximized;
        QRect geometry;
        bool isEmpty() const {return geometry.isEmpty();}
    };

public:
    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialog()
    {
        auto classType = QString::fromUtf8(DialogType::staticMetaObject.className());

        auto finder = [classType](const std::shared_ptr<DialogInfoBase>& dialogInfo) {
            return (dialogInfo->getDialogClass() == classType);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }

    template <class DialogType>
    static void removeDialogByClass()
    {
        auto dialogInfo = findDialog<DialogType>();
        if(dialogInfo)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);
            if(dialogInfoByType)
            {
                removeDialog(dialogInfoByType->getDialog());
            }
        }
    }

    template <class ParentType>
    static void closeDialogsByParentClass()
    {
        auto parentDialog = findDialog<ParentType>();
        if(parentDialog)
        {
            Platform::getInstance()->closeFileFolderSelectors(parentDialog->getDialog());

            foreach(auto dialogInfo, mOpenedDialogs)
            {
                if(dialogInfo->isParent(parentDialog->getDialog()))
                {
                    dialogInfo->close();
                }
            }

            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

    template <class DialogType, class ParentType>
    static void setParent(QPointer<DialogType> dialog, bool whenParentIsActivated)
    {
        auto parentDialog = findDialog<ParentType>();
        if(parentDialog && ((whenParentIsActivated && parentDialog->getDialog()->isActiveWindow())
                            || (!whenParentIsActivated && !parentDialog->getDialog()->isMinimized())))
        {
            closeDialogsByParentClass<ParentType>();

            auto windowFlags = dialog->windowFlags();
            parentDialog->raise(true);
            dialog->setParent(parentDialog->getDialog());
            dialog->setWindowFlags(windowFlags);
        }
    }

    template <class DialogType, class ParentType>
    static void showDialog(QPointer<DialogType> dialog, bool whenParentIsActivated, std::function<void()> func)
    {
        if(dialog)
        {
            setParent<DialogType, ParentType>(dialog,whenParentIsActivated);
            showDialog<DialogType>(dialog, func);
        }
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [func, dialog](){
                if(func)
                {
                    func();
                }
                removeDialog(dialog);
            });
            showDialogImpl(dialog);
        }
    }

    template <class DialogType, class ParentType, class CallbackClass>
    static void showDialog(QPointer<DialogType> dialog, bool whenParentIsActivated, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
    {
        if(dialog)
        {
            setParent<DialogType, ParentType>(dialog, whenParentIsActivated);

            dialog->connect(dialog.data(), &DialogType::finished, [dialog, caller, func](){
                removeDialog(dialog);
                if(caller && func)
                {
                    (caller->*func)(dialog);
                }
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
                if(caller && func)
                {
                    (caller->*func)(dialog);
                }
                removeDialog(dialog);
            });

            showDialogImpl(dialog);
        }
    }

    template <class DialogType>
    static void showNonModalDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            dialog->setModal(false);
            showDialogImpl(dialog, false);
        }
    }

    template <class DialogType>
    static void showGeometryRetainerDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            showDialogImpl(dialog, false);
            auto classType = QString::fromUtf8(DialogType::staticMetaObject.className());
            mSavedGeometries.insert(classType, GeometryInfo());
        }
    }

    template <class DialogType>
    static void showDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            showDialogImpl(dialog);
        }
    }

    template <class DialogType>
    static void removeDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            auto classType = QString::fromUtf8(DialogType::staticMetaObject.className());
            if(mSavedGeometries.contains(classType))
            {
                GeometryInfo info;
                info.maximized = dialog->isMaximized();
                info.geometry = dialog->geometry();
                mSavedGeometries.insert(classType, info);
            }

            if(dialog->parent())
            {
                qApp->setActiveWindow(dialog->parentWidget());
            }

            dialog->deleteLater();
        }
    }

    static void raiseAllDialogs()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            dialogInfo->raise();
        }

        qApp->processEvents();
    }

    static void closeAllDialogs()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            dialogInfo->close();
        }
    }

private:
    static QList<std::shared_ptr<DialogInfoBase>> mOpenedDialogs;
    static QMap<QString, GeometryInfo> mSavedGeometries;

    template <class DialogType>
    static void removeWhenClose(QPointer<DialogType> dialog)
    {
        dialog->connect(dialog.data(), &DialogType::finished, [dialog]()
        {
            removeDialog(dialog);
        });
    }

    template <class DialogType>
    static void showDialogImpl(QPointer<DialogType> dialog, bool changeWindowModality = true)
    {
        if(dialog)
        {
            auto classType = QString::fromUtf8(DialogType::staticMetaObject.className());
            auto info = findSiblingDialogInfo<DialogType>(classType);

            if(info)
            {
                if(info->getDialog() != dialog)
                {
                    QRect siblingGeometry = info->getDialog()->geometry();
                    bool siblingIsMaximized = info->getDialog()->isMaximized();
                    dialog->setWindowFlags(info->getDialog()->windowFlags());
                    removeDialog(info->getDialog());
                    info->setDialog(dialog);
                    info->setDialogClass(classType);

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
                info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);
                dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

                initDialog(dialog);
            }

#ifdef Q_OS_WINDOWS
            ExternalDialogOpener externalOpener;
#endif

            if(changeWindowModality)
            {
                if(dialog->parent())
                {
                    dialog->setWindowModality(Qt::WindowModal);
                }
            }

            auto geoInfo = mSavedGeometries.value(classType, GeometryInfo());
            if(!geoInfo.isEmpty())
            {
                //First time this is used
                if(geoInfo.maximized)
                {
                    auto dialogGeo = dialog->geometry();
                    dialogGeo.moveCenter(geoInfo.geometry.center());
                    dialog->move(dialogGeo.topLeft());
                    dialog->showMaximized();
                }
                else
                {
                    dialog->setGeometry(geoInfo.geometry);
                    dialog->show();
                }
            }
            else
            {
                dialog->show();
            }

            info->raise(true);
        }
    }

    template <class DialogType>
    static void initDialog(QPointer<DialogType> dialog)
    {
        dialog->connect(dialog.data(), &QObject::destroyed, [dialog](){
            auto info = findDialogInfo<DialogType>(dialog);
            if(info)
            {
                mOpenedDialogs.removeOne(info);
            }
        });
        auto dpiResize = new HighDpiResize<DialogType>(dialog);
        Q_UNUSED(dpiResize);
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialogInfo(QPointer<DialogType> dialog)
    {
        auto finder = [dialog](const std::shared_ptr<DialogInfoBase> dialogInfo) {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            return (dialogInfoByType && dialogInfoByType->getDialog() == dialog);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findSiblingDialogInfo(const QString& classType)
    {
        auto finder = [classType](const std::shared_ptr<DialogInfoBase> dialogInfo) {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType && !dialogInfoByType->getDialogClass().isEmpty())
            {
                return (dialogInfoByType->getDialogClass()  == classType);
            }

            return false;
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }
};


#endif // DIALOGOPENER_H
