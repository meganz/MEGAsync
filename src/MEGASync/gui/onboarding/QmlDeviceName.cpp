#include "QmlDeviceName.h"

#include <QHostInfo>

static const QString defaultFactoryBiosName = QString::fromUtf8("To be filled by O.E.M.");

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
    auto deviceName = mDeviceNameRequest->getDeviceName();
    if (deviceName.isEmpty())
    {
        deviceName = QHostInfo::localHostName();
    }

    if(mName != deviceName)
    {
        if(deviceName.contains(defaultFactoryBiosName))
        {
            auto hostName = QHostInfo::localHostName();
            if (!hostName.isEmpty() && mName != hostName)
            {
                setDeviceName(hostName);
                return;
            }
        }

        mName = deviceName;
        emit deviceNameChanged();

        if(mChanging)
        {
            mChanging = false;
            emit deviceNameSet();
        }
    }
}
