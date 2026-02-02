#include "QmlDeviceName.h"

QmlDeviceName::QmlDeviceName(QObject* parent):
    QObject{parent},
    mDeviceNameRequest(UserAttributes::DeviceNames::requestDeviceNames())
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

QString QmlDeviceName::getDeviceName() const
{
    return mName;
}

void QmlDeviceName::setDeviceName(const QString& newName)
{
    if (mName == newName)
    {
        onDeviceNameSet();
    }
    else
    {
        mDeviceNameRequest->setDeviceName(newName);
    }
}

void QmlDeviceName::onDeviceNameSet()
{
    const auto isInit = mName.isEmpty();
    mName = mDeviceNameRequest->getDeviceName();
    emit deviceNameChanged();
    if (!isInit)
    {
        emit deviceNameSetRequestCompleted();
    }
}
