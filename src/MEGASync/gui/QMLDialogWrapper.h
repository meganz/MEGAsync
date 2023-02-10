#ifndef QMLCOMPONENTWRAPPER_H
#define QMLCOMPONENTWRAPPER_H

#include <QQmlComponent>
#include <QDebug>
#include <QQmlEngine>

class QMLComponentWrapper : public QObject
{
public:
    QMLComponentWrapper(QObject* parent = 0);
    ~QMLComponentWrapper();

    virtual QUrl getQmlUrl() = 0;
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
        mWrapper = new Type(parent);
        QQmlEngine* engine = mWrapper->getEngine();
        QQmlComponent qmlComponent(engine);/* = new QQmlComponent(mWrapper->getEngine(), mWrapper);*/
        qmlComponent.loadUrl(mWrapper->getQmlUrl());
        QObject* object = qmlComponent.create();
        engine->setObjectOwnership(object, QQmlEngine::JavaScriptOwnership);
        //object->setParent(mWrapper);
        qDebug()<< object->parent();
        connect(object, SIGNAL(accepted()), this, SLOT(cpp1Slot()));
        connect(object, SIGNAL(rejected()), this, SLOT(dialogClosed()));
        connect(object, SIGNAL(destroyed()), this, SLOT(cpp2Slot()));
    }
    void dialogClosed(){mWrapper->deleteLater();}

private:
    QPointer<Type> mWrapper;
};

#endif // QMLCOMPONENTWRAPPER_H
