#ifndef QML_COMPONENT_WRAPPER_H
#define QML_COMPONENT_WRAPPER_H

#include "DialogOpener.h"
#include "megaapi.h"
#include "QmlDialog.h"
#include "QmlDialogWrapperUtilities.h"
#include "QmlManager.h"
#include "StatsEventHandler.h"

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QEvent>
#include <QPointer>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

#include <iostream>

template<class Type>
class QmlDialogWrapper;

class QMLComponent: public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    virtual ~QMLComponent() = default;

    virtual QUrl getQmlUrl() = 0;

    virtual QList<QObject*> getInstancesFromContext();

    QString contextName() const;

    template<typename DialogType, typename... A>
    static auto showDialog(QmlDialog* parent, A&&... args)
    {
        return getDialog<DialogType>(
            [](auto& dialog)
            {
                return DialogOpener::showDialog(dialog);
            },
            parent,
            std::forward<A>(args)...);
    }

    template<typename DialogType, typename... A>
    static auto addDialog(QmlDialog* parent, A&&... args)
    {
        return getDialog<DialogType>(
            [](auto& dialog)
            {
                return DialogOpener::addDialog(dialog);
            },
            parent,
            std::forward<A>(args)...);
    }

signals:
    void dataReady();

private:
    template<typename DialogType, typename... A>
    static QPointer<QmlDialogWrapper<DialogType>> createOrGetDialog(QmlDialog* parent, A&&... args)
    {
        QPointer<QmlDialogWrapper<DialogType>> dialog(nullptr);
        if (auto dialogInfo = DialogOpener::findDialog<QmlDialogWrapper<DialogType>>())
        {
            dialog = dialogInfo->getDialog();
        }
        else
        {
            dialog = new QmlDialogWrapper<DialogType>(parent, std::forward<A>(args)...);
        }

        return dialog;
    }

    template<typename DialogType, typename Operation, typename... A>
    static auto getDialog(Operation operation, QmlDialog* parent, A&&... args)
    {
        auto dialog(createOrGetDialog<DialogType>(parent, std::forward<A>(args)...));
        return operation(dialog);
    }
};

class QmlDialogWrapperBase : public QWidget
{
    Q_OBJECT

public:
    QmlDialogWrapperBase(QWidget *parent = 0);
    ~QmlDialogWrapperBase();

    Qt::WindowModality windowModality();
    void setWindowModality(Qt::WindowModality modality);
    Qt::WindowFlags windowFlags();
    void setWindowFlags(Qt::WindowFlags flags);
    void setWindowState(Qt::WindowState state);
    void move(const QPoint& point);
    void showMaximized();
    void showNormal();
    void setGeometry(const QRect &geometry);
    QRect geometry();
    bool isMaximized();
    bool isMinimized();
    bool isVisible();
    void hide();
    void show();
    void showSync();
    void close();
    void activateWindow();
    QWindow* windowHandle();
    void raise();
    void removeDialog();
    int minimumWidth();
    int maximumWidth();
    int maximumHeight();
    int minimumHeight();
    QRect rect();
    void update(const QRect& rect);
    void resize(int h, int w);
    void resize(const QSize& size);
    QSize size();

    Q_INVOKABLE int result();
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();

    // Re-bind the inner QML window to its current parent widget's native
    // QWindow as transient parent. Idempotent. No-op if there is no
    // parent widget or the inner window has not been created yet.
    Q_INVOKABLE void attachQmlToParentWindow();

    QPointer<QmlDialog> getQmlWindow() const;

signals:
    void finished(int result);
    void requestClose();
    void accepted();
    void rejected();

public slots:
    void onWindowFinished();

protected:
    QPointer<QmlDialog> mWindow;

private:
    QDialog::DialogCode mResult;
    QTimer mShowDelay;
};

template <class Type>
class QmlDialogWrapper : public QmlDialogWrapperBase
{

public:
    template<typename... A>
    QmlDialogWrapper(QmlDialog* parent, A&&... args):
        QmlDialogWrapper(static_cast<QWidget*>(nullptr), std::forward<A>(args)...)
    {
        if (parent)
        {
            // QML->QML: the QWidget-parent constructor delegated above runs
            // with a null QWidget parent, so its attachToParentWindow path
            // is skipped. Bind the inner QQuickWindow directly to the QML
            // parent here so it still behaves as an embedded modal dialog.
            if (mWindow)
            {
                mWindow->attachToParentWindow(parent);
            }

            const auto parentGeometry = parent->geometry();

            // Set on QmlDialog to use for showWhenCreatedQMLs
            DialogOpener::setParentGeometry(mWindow, parentGeometry);
            DialogOpener::setParentGeometry(this, parentGeometry);
        }
    }

    template <typename... A>
    QmlDialogWrapper(QWidget* parent = nullptr, A&&... args)
        : QmlDialogWrapperBase(parent)
    {
        Q_ASSERT((std::is_base_of<QMLComponent, Type>::value));

        mWrapper = new Type(nullptr, std::forward<A>(args)...);
        QQmlEngine* engine = QmlManager::instance()->getEngine();
        QQmlComponent qmlComponent(engine);
        const auto startTime = QDateTime::currentMSecsSinceEpoch();
        qmlComponent.loadUrl(mWrapper->getQmlUrl());
        QEventLoop eventLoop;

        QMetaObject::Connection connection = QObject::connect(
            &qmlComponent,
            &QQmlComponent::statusChanged,
            [&](QQmlComponent::Status status)
            {
                if (status == QQmlComponent::Ready || status == QQmlComponent::Error)
                {
                    eventLoop.quit();
                }
            });
        qmlComponent.loadUrl(mWrapper->getQmlUrl());

        if (qmlComponent.isLoading())
        {
            eventLoop.exec();
        }

        QObject::disconnect(connection);

        QString message = QString::fromUtf8("Time to load Qml file %1: %2ms Status: %3")
                              .arg(mWrapper->getQmlUrl().toString())
                              .arg(QDateTime::currentMSecsSinceEpoch() - startTime)
                              .arg(qmlComponent.status());
        ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

        if (qmlComponent.isReady())
        {
            QQmlContext* context = new QQmlContext(engine->rootContext(), this);
            QmlManager::instance()->setRootContextProperty(mWrapper);

            // SNC-6567 (Phases 1+4): Bind this dialog's wrapper and the data
            // instances it declares to the CHILD QQmlContext BEFORE the QML
            // tree is built. This is the only delivery channel for per-dialog
            // data into QML now that QmlInstancesManager has been removed
            // (Phase 4). The first binding evaluation inside
            // qmlComponent.create() sees real (non-null) values for the
            // identifiers QML uses to reach the C++ side.
            //
            // Child-context properties also give correct per-dialog isolation
            // when multiple QML dialogs are open simultaneously — unlike
            // setRootContextProperty() (kept above for some legacy globals)
            // which writes to the engine's shared root context.
            {
                auto qmlManager = QmlManager::instance();
                const QString wrapperName = qmlManager->getObjectRootContextName(mWrapper.data());
                if (!wrapperName.isEmpty())
                {
                    context->setContextProperty(wrapperName, mWrapper.data());
                }
                const QList<QObject*> instances = mWrapper->getInstancesFromContext();
                for (QObject* instance: instances)
                {
                    if (!instance)
                    {
                        continue;
                    }
                    const QString instanceName = qmlManager->getObjectRootContextName(instance);
                    if (!instanceName.isEmpty())
                    {
                        context->setContextProperty(instanceName, instance);
                    }
                }
            }

            mWindow = dynamic_cast<QmlDialog*>(qmlComponent.create(context));
            Q_ASSERT(mWindow);

            if (mWindow)
            {
                // Bind the inner QQuickWindow to the parent widget's native
                // window so it behaves as a true child/modal dialog of it.
                // Without this, QML dialogs with a parent show as standalone
                // resizable top-level windows because Qt::WindowModal has no
                // transient parent to apply to.
                //
                // IMPORTANT: for QML->QML the parent widget is a
                // QmlDialogWrapperBase whose own QWidget is never shown; the
                // visible window is its inner QQuickWindow (mWindow). We must
                // use that inner QQuickWindow as transient parent. Otherwise
                // QWidget::createWinId() materializes an empty native window
                // and the WM renders it as a stray frame around the dialog.
                if (QWidget* pw = this->parentWidget())
                {
                    QWindow* parentWindow = nullptr;
                    if (auto* qmlBase = qobject_cast<QmlDialogWrapperBase*>(pw))
                    {
                        // Reach the visible inner QQuickWindow (overridden
                        // windowHandle in QmlDialogWrapperBase returns mWindow).
                        parentWindow = qmlBase->windowHandle();
                    }
                    else if (QWidget* topLevel = pw->window())
                    {
                        topLevel->createWinId();
                        parentWindow = topLevel->windowHandle();
                    }

                    if (parentWindow)
                    {
                        mWindow->attachToParentWindow(parentWindow);
                    }
                }

                mWrapper->setParent(mWindow);

                // SNC-6567 (Phase 4): The previous call to
                //     mWindow->getInstancesManager()->initInstances(mWrapper);
                // has been removed. The QmlInstancesManager class no longer
                // exists — data delivery to QML happens via the child
                // QQmlContext properties registered above (Phase 1) BEFORE
                // qmlComponent.create() runs. Bindings see real values on the
                // first evaluation.

                if (parent)
                {
                    // Set on QmlDialog to use for showWhenCreatedQMLs
                    DialogOpener::setVisualParent(mWindow, parent);
                    DialogOpener::setVisualParent(this, parent);
                }
            }

            connect(mWindow, &QmlDialog::finished, this, [this]()
            {
                QmlDialogWrapperBase::onWindowFinished();
            });

            connect(mWindow, &QmlDialog::accepted, this, [this]()
            {
                accept();
            });

            connect(mWindow, &QmlDialog::rejected, this, [this]()
            {
                reject();
            });

            connect(mWindow, &QmlDialog::accept, this, [this]()
            {
                QmlDialogWrapperBase::accept();
            });

            connect(mWindow, &QmlDialog::reject, this, [this]()
            {
                QmlDialogWrapperBase::reject();
            });

            connect(mWindow, &QQuickWindow::screenChanged, this, [this]()
            {
                QApplication::postEvent(this, new QEvent(QEvent::ScreenChangeInternal));
            });

            mWindow->installEventFilter(MegaSyncApp->getStatsEventHandler());

            QApplication::postEvent(this, new QEvent(QEvent::ScreenChangeInternal));
        }
        else
        {
            /*
            * Errors will be printed respecting the original format (with links to source qml that fails).
            * All errors will be printed, using qDebug() some errors were hidden.
            */
            ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_ERROR, qmlComponent.errorString().toUtf8().constData());
            for(const QString& path : engine->importPathList())
            {
                QString message = QString::fromUtf8("QML import path: ") + path;
                ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());
            }

#ifdef DEBUG
            std::cout << qmlComponent.errorString().toStdString() << std::endl;
#endif
        }
    }

    ~QmlDialogWrapper() = default;

    inline Type* wrapper()
    {
        return mWrapper;
    }

    void setShowWhenCreated()
    {
        connect(mWrapper,
                &Type::dataReady,
                this,
                [this]()
                {
                    mWindow->readyToBeShow();
                });

        // DialogOpener checks this flag on the wrapper widget (the object it
        // handles), so it is set on the wrapper only; nothing reads it on the
        // inner window.
        QmlDialogWrapperUtilities::setShowWhenCreated(this, true);
    }

private:
    QPointer<Type> mWrapper;
};

namespace QmlDialogWrapperUtilities
{
template<typename DialogType>
static QmlDialog* getQmlDialog()
{
    auto dialog = DialogOpener::findDialog<QmlDialogWrapper<DialogType>>();
    if (dialog)
    {
        return dialog->getDialog()->getQmlWindow();
    }

    return nullptr;
}
}

#endif // QML_COMPONENT_WRAPPER_H
