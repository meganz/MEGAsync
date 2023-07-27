#include "ComputerName.h"
#include "UserAttributesRequests/DeviceName.h"


ComputerName::ComputerName(QObject *parent)
    : QObject{parent}
    , mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName())
    , mChanging(false)
{
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &ComputerName::onDeviceNameSet);
    if(mDeviceNameRequest->isAttributeReady())
    {
        onDeviceNameSet();
    }
}

QString ComputerName::getDeviceName()
{
    return mDeviceName;
}

bool ComputerName::setDeviceName(const QString &newName)
{
    mChanging = mDeviceNameRequest->setDeviceName(newName);
    return mChanging;
}

void ComputerName::onDeviceNameSet()
{
    if(mDeviceName != mDeviceNameRequest->getDeviceName())
    {
        mDeviceName = mDeviceNameRequest->getDeviceName();
        emit deviceNameChanged();
        if(mChanging)
        {
            mChanging = false;
            emit deviceNameSet();
        }
    }
}
