#ifndef QMLCOMPONENTWRAPPER_H
#define QMLCOMPONENTWRAPPER_H

#include <QQmlComponent>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>


class QMLComponentWrapper : public QObject
{
public:
    QMLComponentWrapper(QObject* parent = 0);
    ~QMLComponentWrapper();

    virtual QUrl getQmlUrl() = 0;
    virtual QString contextName(){return QString();}
    virtual QVariant contextVariant(){return QVariant();}

    QQmlEngine* getEngine();
};

class QMLDialogWrapperBase : public QObject
{
    Q_OBJECT
public:
    QMLDialogWrapperBase(QObject* parent = 0);
    ~QMLDialogWrapperBase();


private slots:
    void cpp1Slot(){ qDebug()<<"CPP1SLOT FROM QML";};
    void cpp2Slot(){ qDebug()<<"DESTROYED";};
    virtual void dialogClosed() = 0;

};


template <class Type>
class QMLDialogWrapper : public QMLDialogWrapperBase
{

public:
    void close(){}
    QMLDialogWrapper(QObject* parent = nullptr) : QMLDialogWrapperBase(parent)
    {
        Q_ASSERT((std::is_base_of<QMLComponentWrapper, Type>::value));
        mWrapper = new Type(parent);
        QQmlEngine* engine = mWrapper->getEngine();
        QQmlComponent qmlComponent(engine);
        qmlComponent.loadUrl(mWrapper->getQmlUrl());
        QObject* object;
        if(!mWrapper->contextName().isEmpty())
        {
            QQmlContext *context = new QQmlContext(engine->rootContext());
            context->setContextProperty(mWrapper->contextName(), mWrapper);
            object = qmlComponent.create(context);
        }
        else
        {
            object = qmlComponent.create();
        }
        connect(object, SIGNAL(accepted()), this, SLOT(cpp1Slot()));
        connect(object, SIGNAL(rejected()), this, SLOT(dialogClosed()));
        connect(object, SIGNAL(destroyed()), this, SLOT(cpp2Slot()));
    }
    void dialogClosed(){}

private:
    QPointer<Type> mWrapper;
};

#endif // QMLCOMPONENTWRAPPER_H
