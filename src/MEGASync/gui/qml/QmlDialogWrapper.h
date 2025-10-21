#ifndef QML_COMPONENT_WRAPPER_H
#define QML_COMPONENT_WRAPPER_H

#include "DialogOpener.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "QmlDialog.h"
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
    static auto showDialog(A&&... args)
    {
        return getDialog<DialogType>(
            [](auto& dialog)
            {
                return DialogOpener::showDialog(dialog);
            },
            std::forward<A>(args)...);
    }

    template<typename DialogType, typename... A>
    static auto addDialog(A&&... args)
    {
        return getDialog<DialogType>(
            [](auto& dialog)
            {
                return DialogOpener::addDialog(dialog);
            },
            std::forward<A>(args)...);
    }

signals:
    void dataReady();

private:
    template<typename DialogType, typename... A>
    static QPointer<QmlDialogWrapper<DialogType>> createOrGetDialog(A&&... args)
    {
        QPointer<QmlDialogWrapper<DialogType>> dialog(nullptr);
        if (auto dialogInfo = DialogOpener::findDialog<QmlDialogWrapper<DialogType>>())
        {
            dialog = dialogInfo->getDialog();
        }
        else
        {
            dialog = new QmlDialogWrapper<DialogType>(std::forward<A>(args)...);
        }

        return dialog;
    }

    template<typename DialogType, typename Operation, typename... A>
    static auto getDialog(Operation operation, A&&... args)
    {
        auto dialog(createOrGetDialog<DialogType>(std::forward<A>(args)...));
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
};


template <class Type>
class QmlDialogWrapper : public QmlDialogWrapperBase
{

public:

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
            mWindow = dynamic_cast<QmlDialog*>(qmlComponent.create(context));
            Q_ASSERT(mWindow);

            if (mWindow)
            {
                mWrapper->setParent(mWindow);
                mWindow->getInstancesManager()->initInstances(mWrapper);
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
        connect(
            mWrapper,
            &Type::dataReady,
            this,
            [this]()
            {
                mWindow->centerAndRaise();
            },
            Qt::UniqueConnection);
    }

private:
    QPointer<Type> mWrapper;
};

#endif // QML_COMPONENT_WRAPPER_H
