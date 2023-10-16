#include "DeviceName.h"

#include "megaapi.h"
#include "mega/types.h"
#include "MegaApplication.h"
#include "platform/Platform.h"

namespace UserAttributes
{
// DEVICE NAME REQUEST
//
// Flow:
// Ask remote for the device name. If it set, emit it.
// If not, set it using the default name.
// If the name is already taken, add a counter as suffix and increment
// while there are collisions.
// The request will be pending while the entire process has not been completed.
// The request will fail only if we can't complete the process.
// The name is emitted on device name change (for instance after setting it).
//

DeviceName::DeviceName(const QString& userEmail) : AttributeRequest(userEmail),
    mDeviceName(getDefaultDeviceName()),
    mNameSuffix(0),
    mUserChoosenDeviceName(QString())
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                       QString::fromUtf8("Default device name: \"%1\"").arg(mDeviceName)
                       .toUtf8().constData());
}

std::shared_ptr<DeviceName> DeviceName::requestDeviceName()
{
    return UserAttributesManager::instance().requestAttribute<DeviceName>();
}

void DeviceName::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    bool isDeviceNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES);
    if (isDeviceNameRequest)
    {
        switch (incoming_request->getType())
        {
        case mega::MegaRequest::TYPE_GET_ATTR_USER:
        {
            processGetDeviceNameCallback(incoming_request, e);
            break;
        }
        case mega::MegaRequest::TYPE_SET_ATTR_USER:
        {
            processSetDeviceNameCallback(incoming_request, e);
            break;
        }
        }
    }
}

void DeviceName::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_DEVICE_NAMES);
}

AttributeRequest::RequestInfo DeviceName::fillRequestInfo()
{
    std::function<void()> requestFunc = []()
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Requesting device name");
        MegaSyncApp->getMegaApi()->getDeviceName(nullptr, nullptr);
    };
    QSharedPointer<ParamInfo> paramInfo(new ParamInfo(requestFunc, QList<int>()
                                                      << mega::MegaError::API_OK
                                                      << mega::MegaError::API_ENOENT    // Case we have to set it
                                                      << mega::MegaError::API_EEXIST)); // Case where the name is already taken
    ParamInfoMap paramInfoMap({{mega::MegaApi::USER_ATTR_DEVICE_NAMES, paramInfo}});
    RequestInfo ret(paramInfoMap, QMap<int64_t, int>({{mega::MegaUser::CHANGE_TYPE_DEVICE_NAMES,
                                                mega::MegaApi::USER_ATTR_DEVICE_NAMES}}));
    return ret;
}

bool DeviceName::isAttributeReady() const
{
    return !isRequestPending();
}

QString DeviceName::getDeviceName() const
{
    return mDeviceName;
}

QString DeviceName::getDefaultDeviceName()
{
    QString deviceName = Platform::getInstance()->getDeviceName();
    // If empty, use generic one.
    return deviceName.isEmpty() ? tr("My computer") : deviceName;
}

bool DeviceName::setDeviceName(const QString &deviceName)
{
    if(deviceName == mDeviceName)
    {
        return false;
    }

    mUserChoosenDeviceName = deviceName;
    setDeviceNameAttribute();
    return true;
}

void DeviceName::processGetDeviceNameCallback(mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    auto errorCode (e->getErrorCode());

    if (errorCode == mega::MegaError::API_OK)
    {
        mDeviceName = QString::fromUtf8(incoming_request->getName());
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                           QString::fromUtf8("Got device name from remote: \"%1\"").arg(mDeviceName)
                           .toUtf8().constData());
        emit attributeReady(mDeviceName);
    }
    else
    {
        QString errorMsg = QString::fromUtf8(e->getErrorString());
        QString logMsg (QString::fromUtf8("Error getting device name from remote: \"%1\"").arg(errorMsg));
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

        // If the device name is not set on remote, set it
        if (errorCode == mega::MegaError::API_ENOENT)
        {
            setDeviceNameAttribute();
        }
    }
}

void DeviceName::processSetDeviceNameCallback(mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    auto errorCode (e->getErrorCode());
    if  (errorCode == mega::MegaError::API_OK)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Device name successfully set on remote");
        mDeviceName = QString::fromUtf8(incoming_request->getName());
        emit attributeReady(mDeviceName);
        mNameSuffix = 0;
    }
    else
    {
        QString errorMsg = QString::fromUtf8(e->getErrorString());
        QString logMsg (QString::fromUtf8("Error setting device name on remote: \"%1\"").arg(errorMsg));
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

        if (errorCode == mega::MegaError::API_EEXIST)
        {
            // Increment suffix and retry
            mNameSuffix++;
            mDeviceName = (mUserChoosenDeviceName.isEmpty() ? getDefaultDeviceName() : mUserChoosenDeviceName)
                                                           + QString::fromLatin1(" - ") + QString::number(mNameSuffix);
            setDeviceNameAttribute(true);
        }
    }
}

void DeviceName::setDeviceNameAttribute(bool isRetry)
{
    QString deviceNameToSet;
    if(isRetry)
    {
        deviceNameToSet = mDeviceName;
    }
    else
    {
        deviceNameToSet = mUserChoosenDeviceName.isEmpty() ? mDeviceName : mUserChoosenDeviceName;
    }

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       QString::fromUtf8("Setting Device name to \"%1\"").arg(deviceNameToSet)
                       .toUtf8().constData());
    mRequestInfo.mParamInfo[mega::MegaApi::USER_ATTR_DEVICE_NAMES]->setPending(true);
    MegaSyncApp->getMegaApi()->setDeviceName(nullptr, deviceNameToSet.toUtf8().constData());
}
}//end namespace UserAttributes
