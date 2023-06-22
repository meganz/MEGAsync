#include "ComputerName.h"
#include "UserAttributesRequests/DeviceName.h"


ComputerName::ComputerName(QObject *parent)
    : QObject{parent}
    , mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName())
    , mRequesting(true)
{
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &ComputerName::onDeviceNameSet);
    if(mDeviceNameRequest->isAttributeReady())
    {
        mRequesting = false;
        onDeviceNameSet();
    }
}

QString ComputerName::getDeviceName()
{
    return mDeviceName;
}

bool ComputerName::setDeviceName(const QString &newName)
{
    return mDeviceNameRequest->setDeviceName(newName);
}

void ComputerName::onDeviceNameSet()
{
    if(mDeviceName != mDeviceNameRequest->getDeviceName())
    {
        mDeviceName = mDeviceNameRequest->getDeviceName();
        emit deviceNameChanged();
    }
}
