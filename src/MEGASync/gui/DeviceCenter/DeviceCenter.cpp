#include "DeviceCenter.h"

static bool qmlRegistrationDone = false;

DeviceCenter::DeviceCenter(QObject* parent):
    QMLComponent(parent)
{
    registerQmlModules();
}

QUrl DeviceCenter::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/deviceCenter/DeviceCenterDialog.qml"));
}

QString DeviceCenter::contextName()
{
    return QString::fromUtf8("deviceCenterAccess");
}

void DeviceCenter::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("DeviceCenter", 1, 0);
        qmlRegisterType<QmlDialog>("DeviceCenterQmlDialog", 1, 0, "DeviceCenterQmlDialog");
        qmlRegistrationDone = true;
    }
}
