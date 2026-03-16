#include "QmlDeviceName.h"

QmlDeviceName::QmlDeviceName(QObject* parent):
    QObject{parent},
    mSetDeviceNameRequested(false),
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
    mSetDeviceNameRequested = true;

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
    mName = mDeviceNameRequest->getDeviceName();
    emit deviceNameChanged();

    // Emit only if this object is the source of the device name set
    if (mSetDeviceNameRequested)
    {
        mSetDeviceNameRequested = false;
        emit deviceNameSetRequestCompleted();
    }
}
