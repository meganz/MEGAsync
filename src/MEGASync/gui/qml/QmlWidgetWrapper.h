#ifndef QML_WIDGET_WRAPPER_H
#define QML_WIDGET_WRAPPER_H

#include "QmlManager.h"

#include <QApplication>
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>

class QMLWidgetComponent: public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    virtual ~QMLWidgetComponent() = default;

    virtual QUrl getQmlUrl() = 0;

    virtual QString contextName() const;
    virtual QList<QObject*> getInstancesFromContext();
};

class QmlWidgetWrapperBase: public QQuickWidget
{
    Q_OBJECT

public:
    QmlWidgetWrapperBase(QWidget* parent = 0);
    ~QmlWidgetWrapperBase();

protected:
    // SNC-6567: was QPointer<QmlItem>; QmlItem was removed because, after the
    // QmlInstancesManager cleanup, it was an empty QQuickItem subclass with
    // no state or behaviour of its own.
    QPointer<QQuickItem> mItem;
};

template<class Type>
class QmlWidgetWrapper: public QmlWidgetWrapperBase
{
public:
    template<typename... A>
    QmlWidgetWrapper(QWidget* parent, A&&... args):
        QmlWidgetWrapperBase(parent)
    {
        Q_ASSERT((std::is_base_of<QMLWidgetComponent, Type>::value));
        mWrapper = new Type(parent, std::forward<A>(args)...);
        if (!parent)
        {
            mWrapper->setParent(this);
        }

        // SNC-6567 (Phases 2b+4): Register the widget (this), the wrapper and
        // the wrapper's declared instances on the engine's root QQmlContext
        // BEFORE setSource() loads the QML. The first binding evaluation that
        // happens inside setSource() then sees real (non-null) values for the
        // identifiers QML uses to reach the C++ side. Mirrors Phase 1 in
        // QmlDialogWrapper.h but for widgets.
        //
        // QQuickWidget doesn't allow injecting a custom child QQmlContext, so
        // properties go on the engine's root context. This is per-process
        // global pollution — acceptable here because these widgets are
        // typically instantiated one at a time. Multi-instance scenarios
        // would require switching from QQuickWidget to a manual QQmlComponent
        // setup, which is out of scope for this phase.
        //
        // This is the only delivery channel for data into widget-side QML now
        // that QmlInstancesManager has been removed (Phase 4).
        {
            auto qmlManager = QmlManager::instance();
            QQmlContext* rootContext = engine()->rootContext();

            const QString widgetName = qmlManager->getObjectRootContextName(this);
            if (!widgetName.isEmpty())
            {
                rootContext->setContextProperty(widgetName, this);
            }
            const QString wrapperName = qmlManager->getObjectRootContextName(mWrapper.data());
            if (!wrapperName.isEmpty())
            {
                rootContext->setContextProperty(wrapperName, mWrapper.data());
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
                    rootContext->setContextProperty(instanceName, instance);
                }
            }
        }

        setSource(mWrapper->getQmlUrl());

        // SNC-6567: rootObject() already returns QQuickItem*, no cast needed
        // (was previously dynamic_cast<QmlItem*>).
        mItem = this->rootObject();
        Q_ASSERT(mItem);

        // SNC-6567 (Phase 4): The previous calls to
        //     mItem->getInstancesManager()->setInstance(this);
        //     mItem->getInstancesManager()->initInstances(mWrapper);
        // have been removed. The QmlInstancesManager class no longer exists —
        // data delivery to QML happens via the root QQmlContext properties
        // registered above (Phase 2b) BEFORE setSource() runs.

        QApplication::postEvent(this, new QEvent(QEvent::ScreenChangeInternal));
    }

    ~QmlWidgetWrapper()
    {
        if (mWrapper && !mWrapper->parent())
        {
            mWrapper->deleteLater();
        }
    }

    inline Type* wrapper()
    {
        return mWrapper;
    }

private:
    QPointer<Type> mWrapper;
};

#endif // QML_WIDGET_WRAPPER_H
