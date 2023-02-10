#include "QMLDialogWrapper.h"
#include "MegaApplication.h"


QMLComponentWrapper::QMLComponentWrapper(QObject *parent)
    : QObject(parent)
{

}

QMLComponentWrapper::~QMLComponentWrapper()
{

}

QQmlEngine *QMLComponentWrapper::getEngine()
{
    return MegaSyncApp->qmlEngine();
}

QMLDialogWrapperBase::QMLDialogWrapperBase(QObject *parent)
    : QObject(parent)
{

}

QMLDialogWrapperBase::~QMLDialogWrapperBase()
{
qDebug()<<"QMLDialogWrapperBase deleted";
}
