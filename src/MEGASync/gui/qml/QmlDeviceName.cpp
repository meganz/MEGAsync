#include "QmlDeviceName.h"

QmlDeviceName::QmlDeviceName(QObject* parent):
    QObject{parent},
    mDeviceNameRequest(UserAttributes::DeviceNames::requestDeviceName()),
    mChanging(false)
{
    connect(mDeviceNameRequest.get(),
            &UserAttributes::DeviceNames::attributeReady,
            this,
            &QmlDeviceName::onDeviceNameSet,
            Qt::QueuedConnection);

    if (mDeviceNameRequest->isAttributeReady())
    {
        onDeviceNameSet();
    }
}

QString QmlDeviceName::getDeviceName()
{
    return mName;
}

bool QmlDeviceName::setDeviceName(const QString& newName)
{
    mChanging = mDeviceNameRequest->setDeviceName(newName);
    return mChanging;
}

void QmlDeviceName::onDeviceNameSet()
{
    mName = mDeviceNameRequest->getDeviceName();
    emit deviceNameChanged();

    if (mChanging)
    {
        mChanging = false;
        emit deviceNameSet();
    }
}
