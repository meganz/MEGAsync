#include "DeviceName.h"

#include "megaapi.h"
#include "mega/types.h"
#include "MegaApplication.h"
#include "platform/Platform.h"

namespace UserAttributes
{
// DEVICE NAME REQUEST
//
//
DeviceName::DeviceName(const QString& userEmail) : AttributeRequest(userEmail),
    mIsDeviceNameSetOnRemote(false),
    mIsRequestFinished(true),
    mDeviceName(getDefaultDeviceName()),
    mNameSuffix(0)
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                       QString::fromUtf8("Default device name: \"%1\"").arg(mDeviceName)
                       .toUtf8().constData());
}

void DeviceName::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    bool isDeviceNameNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES);

    if (isDeviceNameNameRequest)
    {
        int errorCode (e->getErrorCode());

        switch (incoming_request->getType())
        {
        case mega::MegaRequest::TYPE_GET_ATTR_USER:
        {
            if (errorCode == mega::MegaError::API_OK)
            {
                mDeviceName = QString::fromUtf8(incoming_request->getName());
                mIsDeviceNameSetOnRemote = true;
                mIsRequestFinished = true;
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                   QString::fromUtf8("Got device name from remote: \"%1\"").arg(mDeviceName)
                                   .toUtf8().constData());
                emit attributeReady(mDeviceName);
            }
            else
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                                   QString::fromUtf8("Device name not set on remote.")
                                   .toUtf8().constData());
                setDeviceNameAttribute();
            }
            break;
        }
        case mega::MegaRequest::TYPE_SET_ATTR_USER:
        {
            if (errorCode == mega::MegaError::API_OK)
            {
                mIsDeviceNameSetOnRemote = true;
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Device name successfully set on remote");
            }
            else
            { // If the set fails, false mIsDeviceNameSetOnRemote will force a retry next time
                // getDeviceName() is called.
                mIsDeviceNameSetOnRemote = false;
                QString errorMsg = QString::fromUtf8(e->getErrorString());
                QString logMsg (QString::fromUtf8("Error setting device name on remote: \"%1\"").arg(errorMsg));
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

                if (errorCode == mega::MegaError::API_EEXIST)
                {
                    // Increment suffix and retry
                    mNameSuffix++;
                    mDeviceName = getDefaultDeviceName() + QString::fromLatin1(" - ") + QString::number(mNameSuffix);
                    setDeviceNameAttribute();
                }
            }
            mIsRequestFinished = true;
            break;
        }
        }
    }
}

AttributeRequest::RequestInfo DeviceName::fillRequestInfo()
{
    std::function<void()> requestFunc = [this]()
    {
        if (mIsRequestFinished && !mIsDeviceNameSetOnRemote)
        {
            mIsRequestFinished = false;
            QString logMsg (QLatin1String("Requesting device name"));
//            if (forceRefresh)
//            {
//                logMsg += QLatin1String(" (forced)");
//            }
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, logMsg.toUtf8().constData());
            MegaSyncApp->getMegaApi()->getDeviceName();
        }
    };
    QSharedPointer<ParamInfo> paramInfo(new ParamInfo(requestFunc, QList<int>()
                                                      << mega::MegaError::API_OK));
    ParamInfoMap paramInfoMap({{mega::MegaApi::USER_ATTR_DEVICE_NAMES, paramInfo}});
    RequestInfo ret(paramInfoMap, QMap<int, int>({{mega::MegaUser::CHANGE_TYPE_DEVICE_NAMES,
                                                mega::MegaApi::USER_ATTR_DEVICE_NAMES}}));
    return ret;
}

void DeviceName::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_DEVICE_NAMES);
}

void DeviceName::setDeviceNameAttribute()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       QString::fromUtf8("Setting Device name to \"%1\"").arg(mDeviceName)
                       .toUtf8().constData());
    MegaSyncApp->getMegaApi()->setDeviceName(mDeviceName.toUtf8().constData());
}

QString DeviceName::getDeviceName()
{
//    requestDeviceNameAttribute(false);
    return mDeviceName;
}

QString DeviceName::getDefaultDeviceName()
{
    QString deviceName = Platform::getDeviceName();
    if (deviceName.isEmpty())
    {
        // If empty, use generic one.
        deviceName = tr("Your computer");
    }
    return deviceName;
}

bool DeviceName::isAttributeReady() const
{
    return (mIsDeviceNameSetOnRemote && mIsRequestFinished);
}

std::shared_ptr<DeviceName> DeviceName::requestDeviceName()
{
    return UserAttributesManager::instance().requestAttribute<DeviceName>();
}

}//end namespace UserAttributes
