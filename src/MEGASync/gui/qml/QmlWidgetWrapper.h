#ifndef QML_WIDGET_WRAPPER_H
#define QML_WIDGET_WRAPPER_H

#include "megaapi.h"
#include "QmlItem.h"
#include "QmlManager.h"

#include <QApplication>
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QQuickWindow>

class QMLWidgetComponent: public QObject
{
public:
    using QObject::QObject;
    virtual ~QMLWidgetComponent() = default;

    virtual QUrl getQmlUrl() = 0;

    virtual QString contextName() const;
    virtual QList<QObject*> getInstances();
};

class QmlWidgetWrapperBase: public QQuickWidget
{
    Q_OBJECT

public:
    QmlWidgetWrapperBase(QWidget* parent = 0);
    ~QmlWidgetWrapperBase();

protected:
    QPointer<QmlItem> mItem;
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

        setSource(mWrapper->getQmlUrl());

        mItem = dynamic_cast<QmlItem*>(this->rootObject());
        Q_ASSERT(mItem);

        if (mItem)
        {
            mItem->getInstancesManager()->setInstance(this);
            mItem->getInstancesManager()->initInstances(mWrapper);
        }

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
