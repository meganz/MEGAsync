#include "QmlDeviceName.h"

QmlDeviceName::QmlDeviceName(QObject *parent)
    : QObject{parent}
    , mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName())
    , mChanging(false)
{
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &QmlDeviceName::onDeviceNameSet);
    if(mDeviceNameRequest->isAttributeReady())
    {
        onDeviceNameSet();
    }
}

QString QmlDeviceName::getDeviceName()
{
    return mName;
}

bool QmlDeviceName::setDeviceName(const QString &newName)
{
    mChanging = mDeviceNameRequest->setDeviceName(newName);
    return mChanging;
}

void QmlDeviceName::onDeviceNameSet()
{
    if(mName != mDeviceNameRequest->getDeviceName())
    {
        mName = mDeviceNameRequest->getDeviceName();
        emit deviceNameChanged();
        if(mChanging)
        {
            mChanging = false;
            emit deviceNameSet();
        }
    }
}
