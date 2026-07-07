#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include "HighDpiResize.h"
#include "megaapi.h"
#include "Platform.h"
#include "QmlDialogWrapperUtilities.h"
#include "TokenParserWidgetManager.h"

#include <QApplication>
#include <QDialog>
#include <QMap>
#include <QMessageBox>
#include <QPointer>
#include <QQueue>
#include <QRect>
#include <QWindow>

#include <functional>
#include <memory>

// Needed to store a QPointer<QWidget> in a dynamic property (QVariant) so the
// visual-parent reference nulls automatically when the parent is destroyed.
Q_DECLARE_METATYPE(QPointer<QWidget>)

template<typename T>
class QmlDialogWrapper;

class MessageDialogComponent;
class MessageDialogData;

#ifdef Q_OS_WINDOWS
class ExternalDialogOpener : public QWidget
{
    Q_OBJECT

public:
    ExternalDialogOpener();
    ~ExternalDialogOpener();
};
#endif

class DialogBlocker : public QDialog
{
    Q_OBJECT

public:
    DialogBlocker(QWidget* parent);
    ~DialogBlocker();
};

// Wayland-safe activation for plain QWidget dialogs. QWidget::activateWindow()
// forwards to QWindow::requestActivate(), which Wayland does not support: it
// only logs "Wayland does not support QWindow::requestActivate()". Request user
// attention via alert() there instead. QmlDialogWrapper already does this
// internally (see QmlDialogWrapperBase::activateWindow()), so callers must skip
// this helper for QML dialogs and let the wrapper's own override run.
// TODO Qt6: call activateWindow() unconditionally — Qt6 activates via
// xdg-activation without warning.
inline void activateWidgetWaylandSafe(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    if (Platform::getInstance()->isWayland())
    {
        QApplication::alert(widget);
    }
    else
    {
        widget->activateWindow();
    }
}

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

        virtual bool sameDialog(QObject* check) const = 0;
        virtual void raise(bool raiseIfMinimized = false) = 0;
        virtual void show() = 0;
        virtual void close() = 0;
        virtual bool isVisible() = 0;
        virtual bool isActive() = 0;
        virtual bool isParent(QObject* parent) = 0;
        virtual QRect frameGeometry() const = 0;
        virtual void applyCurrentTheme() = 0;

        bool ignoreCloseAllAction() const {return mIgnoreCloseAllAction;}
        void setIgnoreCloseAllAction(bool newIgnoreCloseAllAction){mIgnoreCloseAllAction = newIgnoreCloseAllAction;}

        bool ignoreRaiseAllAction() const {return mIgnoreRaiseAllAction;}
        void setIgnoreRaiseAllAction(bool newIgnoreRaiseAllAction){mIgnoreRaiseAllAction = newIgnoreRaiseAllAction;}

    protected:
        QString mDialogClass;
        bool mIgnoreCloseAllAction = false;
        bool mIgnoreRaiseAllAction = false;
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

        bool sameDialog(QObject* check) const override
        {
            return mDialog == check;
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
                if (QmlDialogWrapperUtilities::isQML(mDialog->windowHandle()))
                {
                    // QmlDialogWrapper::activateWindow() is already Wayland-safe.
                    mDialog->activateWindow();
                }
                else
                {
                    activateWidgetWaylandSafe(mDialog.data());
                }
            }
        }

        void show() override
        {
            if (!QmlDialogWrapperUtilities::isQML(mDialog->windowHandle()) &&
                Platform::getInstance()->isWayland())
            {
                // Plain QWidget dialog on Wayland: setWindowState(WindowActive)
                // triggers the unsupported QWindow::requestActivate(). QML
                // dialogs use the wrapper's own Wayland-safe setWindowState().
                QApplication::alert(mDialog.data());
            }
            else
            {
                mDialog->setWindowState(Qt::WindowActive);
            }
        }

        bool isVisible() override
        {
            return mDialog->isVisible();
        }

        bool isActive() override
        {
            return mDialog->isActiveWindow();
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

        QRect frameGeometry() const override
        {
            return mDialog->frameGeometry();
        }

        bool operator==(const DialogInfo &info)
        {
            return (info.mDialog == mDialog);
        }

        void applyCurrentTheme() override
        {
            Platform::getInstance()->applyCurrentThemeOnCurrentDialogFrame(mDialog->windowHandle());
        }

    private:
        QPointer<DialogType> mDialog;
    };

    struct GeometryInfo
    {
        bool maximized = false;
        QRect geometry;
        bool isEmpty() const {return geometry.isEmpty();}
    };

    static constexpr const char* PARENT_GEOMETRY_PROPERTY = "ParentGeometry";
    static constexpr const char* VISUAL_PARENT_PROPERTY = "VisualParent";

public:
    static QPoint initialDialogPosition(const QSize& dialogSize)
    {
        return Platform::getInstance()->initialDialogPosition(dialogSize);
    }

    static QPoint initialDialogPosition(const QSize& dialogSize, const QRect& parentGeometry)
    {
        return Platform::getInstance()->initialDialogPosition(dialogSize, parentGeometry);
    }

    // Remembers, on a dialog, the geometry of the window it should be centered
    // on. Stored as a dynamic property so it works for any dialog type (QML
    // wrappers and plain QWidget/QDialog alike) and, crucially, lets a dialog be
    // centered on a window WITHOUT making that window its Qt parent (which would
    // couple modality and lifetime). Read back when positioning the dialog.
    // For plain QWidget dialogs, prefer setVisualParent() instead — it derives
    // the geometry live and also supplies the QWindow* needed on Wayland.
    static void setParentGeometry(QObject* dialog, const QRect& parentGeometry)
    {
        if (dialog)
        {
            dialog->setProperty(PARENT_GEOMETRY_PROPERTY, parentGeometry);
        }
    }

    static QRect getParentGeometry(const QObject* dialog)
    {
        if (!dialog)
        {
            return QRect();
        }

        const auto value(dialog->property(PARENT_GEOMETRY_PROPERTY));
        return value.isValid() ? value.toRect() : QRect();
    }

    // Stores the visual parent widget so showDialogImpl() can derive both the
    // centering geometry and (on Wayland) the transient-parent window handle
    // from a single source — without adding Wayland-specific storage.
    // Use this instead of setParentGeometry() whenever a QWidget* is available.
    static void setVisualParent(QObject* dialog, QWidget* parentWidget)
    {
        if (dialog && parentWidget)
        {
            // Stored as a QPointer so it nulls automatically if the parent window
            // is destroyed before the dialog is shown. The dialog is deliberately
            // NOT Qt-parented to the visual parent (see setParentGeometry's note on
            // decoupling lifetime), so their lifetimes are independent; a raw
            // pointer would dangle and crash getVisualParent()/showDialogImpl().
            dialog->setProperty(VISUAL_PARENT_PROPERTY,
                                QVariant::fromValue(QPointer<QWidget>(parentWidget->window())));
        }
    }

    static QWidget* getVisualParent(const QObject* dialog)
    {
        if (!dialog)
        {
            return nullptr;
        }
        return dialog->property(VISUAL_PARENT_PROPERTY).value<QPointer<QWidget>>().data();
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialog()
    {
        auto classType = className<DialogType>();

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

    typedef QmlDialogWrapper<MessageDialogComponent> QmlMessageDialogWrapper;
    static void showMessageDialog(QPointer<QmlMessageDialogWrapper> wrapper,
                                  QPointer<MessageDialogData> msgInfo);

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
        }
    }

    template<class DialogType>
    static void closeDialogsByClass()
    {
        auto dialog = findDialog<DialogType>();
        if (dialog)
        {
            dialog->close();
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

            parentDialog->raise(true);
            dialog->setParent(parentDialog->getDialog(), dialog->windowFlags());
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
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [func, dialog](){
                removeDialog(dialog);
                if(func)
                {
                    func();
                }
            });
            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType, class ParentType, class CallbackClass>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, bool whenParentIsActivated, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
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

            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType, class CallbackClass>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
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

            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showNonModalDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            dialog->setModal(false);
            return showDialogImpl(dialog, false);
        }
        return nullptr;
    }

    template <class DialogType>
    static void showGeometryRetainerDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            showDialogImpl(dialog, false);
            auto classType = className<DialogType>();
            mSavedGeometries.insert(classType, GeometryInfo());
        }
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog)
    {
        std::shared_ptr<DialogInfo<DialogType>> ret = nullptr;
        if(dialog)
        {
            removeWhenClose(dialog);
            ret = showDialogImpl(dialog);
        }
        return ret;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> addDialog(QPointer<DialogType> dialog)
    {
        std::shared_ptr<DialogInfo<DialogType>> ret = nullptr;

        if(dialog)
        {
            QString classType = className<DialogType>();
            auto info = findSiblingDialogInfo<DialogType>(classType);

            if(!info)
            {
                info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);
                initDialog(dialog);
                removeWhenClose(dialog);
            }
            ret = info;
        }
        return ret;
    }

    template <class DialogType>
    static void removeDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            auto classType = className<DialogType>();
            if(mSavedGeometries.contains(classType))
            {
                GeometryInfo info;
                info.maximized = dialog->isMaximized();
                info.geometry = dialog->geometry();
                mSavedGeometries.insert(classType, info);
            }

            if(dialog->parent())
            {
                activateWidgetWaylandSafe(dialog->parentWidget());
            }

            dialog->deleteLater();
        }
    }

    static void raiseAllDialogs()
    {
        bool anyRaised = false;
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(!dialogInfo->ignoreRaiseAllAction())
            {
                dialogInfo->raise();
                anyRaised = true;
            }
        }

        if(anyRaised)
        {
            qApp->processEvents();
        }
    }

    static void closeAllDialogs()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(!dialogInfo->ignoreCloseAllAction())
            {
                dialogInfo->close();
            }
        }
    }

    static void currentThemeChanged()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            dialogInfo->applyCurrentTheme();
        }
    }

    static bool anyVisibleAndActiveDialogs()
    {
        return std::any_of(mOpenedDialogs.begin(),
                           mOpenedDialogs.end(),
                           [](std::shared_ptr<DialogInfoBase> dialogInfo)
                           {
                               return dialogInfo->isVisible() && dialogInfo->isActive();
                           });
    }

    static bool isAnyDialogVisible()
    {
        return std::any_of(mOpenedDialogs.begin(),
                           mOpenedDialogs.end(),
                           [](std::shared_ptr<DialogInfoBase> dialogInfo)
                           {
                               return dialogInfo->isVisible();
                           });
    }

    // True if any currently-open dialog is parented to the given object.
    // Qt5-ONLY: added solely for the InfoDialog Wayland anchoring workaround —
    // keep the InfoDialog mapped only while it parents a child dialog (add
    // sync, add backup, ...), not for unrelated top-level dialogs.
    // TODO Qt6: remove — Qt6's Wayland backend makes the workaround unnecessary.
    static bool isAnyDialogChildOf(QObject* parent)
    {
        return std::any_of(mOpenedDialogs.cbegin(),
                           mOpenedDialogs.cend(),
                           [parent](const std::shared_ptr<DialogInfoBase>& info)
                           {
                               return info->isParent(parent);
                           });
    }

    static QList<QPointer<QWidget>> getAllOpenedDialogs();

    // When a QQuickWindow is closed or destroyed while other QML windows are
    // visible, the remaining windows can stay blank until the next input
    // event forces a frame (reproduced on macOS with both Qt5 and Qt6).
    // Schedule an update on the opened QML dialogs so they repaint
    // immediately.
    static void refreshOtherQmlWindows(QWindow* excludedWindow = nullptr);

private:
    static QList<std::shared_ptr<DialogInfoBase>> mOpenedDialogs;
    static QQueue<std::shared_ptr<DialogInfoBase>> mDialogsQueue;
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
    static std::shared_ptr<DialogInfo<DialogType>> showDialogImpl(QPointer<DialogType> dialog, bool changeWindowModality = true, bool removeSiblings = true)
    {
        if(dialog)
        {
            QString classType = className<DialogType>();
            auto info = findSiblingDialogInfo<DialogType>(classType);

            bool isQML(QmlDialogWrapperUtilities::isQML(dialog->windowHandle()));

            bool ignoreGeometry(isQML && QmlDialogWrapperUtilities::isShowWhenCreated(dialog));
            QRect geometry;
            QByteArray siblingGeometryState;

            if(info)
            {
                if(removeSiblings && info->getDialog() != dialog)
                {
                    siblingGeometryState = info->getDialog()->saveGeometry();
                    dialog->setWindowFlags(info->getDialog()->windowFlags());
                    removeDialog(info->getDialog());
                    info->setDialog(dialog);
                    info->setDialogClass(classType);

                    initDialog(dialog);
                }
                else if (info->getDialog() == dialog)
                {
                    ignoreGeometry = true;
                }
            }
            else
            {
                info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);

                initDialog(dialog);
            }

#ifdef Q_OS_WINDOWS
            ExternalDialogOpener externalOpener;
#endif

            if (!dialog)
            {
                ::mega::MegaApi::log(
                    ::mega::MegaApi::LOG_LEVEL_ERROR,
                    QString::fromUtf8("DialogOpener: Dialog %1 removed while being opened.")
                        .arg(classType)
                        .toUtf8()
                        .constData());

                return nullptr;
            }

            if (!isQML)
            {
                TokenParserWidgetManager::instance()->applyCurrentTheme(dialog);
            }

            // Use to reload the widget stylesheet. Without this line, the new stylesheet is not
            // correctly applied.
            dialog->setParent(dialog->parentWidget(), dialog->windowFlags());

            if (dialog->parent() && changeWindowModality)
            {
                dialog->setWindowModality(Qt::WindowModal);
            }

            // For QML dialog wrappers, the setParent() above can recreate the
            // native handle and drop the inner QQuickWindow's transient parent
            // binding. Re-bind it via the public slot.
            if (isQML && dialog && dialog->parent())
            {
                QMetaObject::invokeMethod(dialog, "attachQmlToParentWindow", Qt::DirectConnection);
            }

            if (ignoreGeometry)
            {
                dialog->show();
            }
            else
            {
                QRect parentGeo;
                QWindow* visualParentWindow = nullptr;
                // Prefer a stored visual parent (setVisualParent): derives the
                // centering geometry live and — for non-QML dialogs on Wayland —
                // also supplies the transient-parent window handle so the
                // compositor can centre the dialog itself.
                // Falls back to an explicit QRect (setParentGeometry, used by the
                // QML→QML path where only a QQuickWindow* is available) or the
                // Qt parent's top-level frame. Geometry-retaining dialogs
                // (NodeSelector, TransferManager, StalledIssuesDialog) have none
                // of the above and keep restoring their saved geometry.
                if (QWidget* visualParent = getVisualParent(dialog))
                {
                    parentGeo = visualParent->frameGeometry();
                    if (!isQML)
                    {
                        visualParentWindow = visualParent->windowHandle();
                    }
                }
                else if (isQML)
                {
                    parentGeo = getParentGeometry(dialog);
                }
                else
                {
                    // Non-QML without visual parent
                    parentGeo = getParentGeometry(dialog);
                    if (!parentGeo.isValid())
                    {
                        if (QWidget* parentWidget = dialog->parentWidget())
                        {
                            parentGeo = parentWidget->window()->frameGeometry();
                        }
                    }
                }

                if (parentGeo.isValid())
                {
                    QWindow* qmlWindow = isQML ? dialog->windowHandle() : nullptr;
                    if (qmlWindow)
                    {
                        // QML dialogs: the visible window is the inner QQuickWindow,
                        // not the wrapper QWidget. Moving the wrapper mis-converts
                        // the coordinates on high-DPI secondary monitors (the wrapper
                        // and the QML window can be bound to different screens/DPR, so
                        // the logical->native conversion lands the window off-screen).
                        // Position the QML window directly, binding it to the target
                        // screen first so the conversion uses the right DPI.
                        const QPoint targetPos =
                            initialDialogPosition(qmlWindow->size(), parentGeo);
                        QmlDialogWrapperUtilities::bindToScreenForPositioning(qmlWindow,
                                                                              parentGeo.center(),
                                                                              targetPos);
                        qmlWindow->setFramePosition(targetPos);
                        dialog->show();
                    }
                    else
                    {
                        Platform::getInstance()->moveDialog(
                            dialog,
                            initialDialogPosition(dialog->geometry().size(), parentGeo),
                            visualParentWindow);
                        dialog->show();
                    }
                }
                else
                {
                    auto savedGeo = mSavedGeometries.value(classType, GeometryInfo());
                    if (!savedGeo.isEmpty())
                    {
                        geometry = savedGeo.geometry;
                    }

                    if (!dialog->isVisible())
                    {
                        if (!siblingGeometryState.isEmpty())
                        {
                            // Reopening over a still-open sibling: restoreGeometry
                            // round-trips the frame position exactly on every platform,
                            // avoiding the upward drift that setGeometry() causes on macOS.
                            dialog->restoreGeometry(siblingGeometryState);
                            dialog->show();
                        }
                        else if (geometry.isValid())
                        {
                            // First time this is used
                            if (savedGeo.maximized)
                            {
                                dialog->showMaximized();
                            }
                            else
                            {
                                dialog->setGeometry(geometry);
                                dialog->show();
                            }
                        }
                        else
                        {
                            auto pos(initialDialogPosition(dialog->geometry().size()));
                            auto size(dialog->geometry().size());
                            dialog->setGeometry(
                                QRect(pos.x(), pos.y(), size.width(), size.height()));
                            dialog->show();
                        }
                    }
                }
            }

            if (dialog->parent())
            {
                auto parentInfo = findDialogInfo(dialog->parent());
                if (parentInfo)
                {
                    parentInfo->raise(true);
                }
            }

            info->raise(true);

            // A QML dialog shown through the "ignoreGeometry" path (registered via
            // addDialog() before showDialog(), e.g. the guest dialog) never has a
            // geometry committed, so its freshly created QQuickWindow can stay at
            // its initial 1x1 size (the QML-declared width/height are not flushed
            // to the native window until a geometry is set). Make sure the window
            // is never shown smaller than its own minimum size.
            if (isQML && (dialog->geometry().width() < dialog->minimumWidth() ||
                          dialog->geometry().height() < dialog->minimumHeight()))
            {
                dialog->resize(dialog->minimumWidth(), dialog->minimumHeight());
            }

            Platform::getInstance()->applyCurrentThemeOnCurrentDialogFrame(dialog->windowHandle());

            return info;
        }

        return nullptr;
    }

    template <class DialogType>
    static void initDialog(QPointer<DialogType> dialog)
    {
        // Evaluated now: inside the destroyed lambda the QPointer is already
        // null, so the dialog type cannot be queried there.
        const bool isQML(QmlDialogWrapperUtilities::isQML(dialog->windowHandle()));

        if (isQML)
        {
            // Repaint the remaining QML windows when this one leaves the
            // screen: that is the moment their content can get invalidated
            // and stay blank until the next input event.
            QWindow* window = dialog->windowHandle();
            QObject::connect(window,
                             &QWindow::visibleChanged,
                             window,
                             [window](bool visible)
                             {
                                 if (!visible)
                                 {
                                     refreshOtherQmlWindows(window);
                                 }
                             });
        }

        dialog->connect(dialog.data(),
                        &QObject::destroyed,
                        [dialog, isQML]()
                        {
                            auto info = findDialogInfo<DialogType>(dialog);
                            if (info)
                            {
                                mOpenedDialogs.removeOne(info);
                            }

                            if (isQML)
                            {
                                // The wrapper's inner QQuickWindow is deleted right after
                                // this (deleteLater from the wrapper's destructor): refresh
                                // the remaining QML dialogs so they repaint. No window to
                                // exclude; this dialog's entry was just removed.
                                refreshOtherQmlWindows();
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

    static std::shared_ptr<DialogInfoBase> findDialogInfo(QObject* dialog)
    {
        auto finder = [dialog](const std::shared_ptr<DialogInfoBase> dialogInfo)
        {
            return dialogInfo->sameDialog(dialog);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return (*itOccurence);
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

    template <class DialogType>
    static QString className()
    {
        return QString::fromUtf8(typeid(DialogType).name());
    }
};

#endif // DIALOGOPENER_H
