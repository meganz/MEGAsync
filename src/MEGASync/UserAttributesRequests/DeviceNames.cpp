#include "DeviceNames.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "Platform.h"

namespace UserAttributes
{
// DEVICE NAME REQUEST
//
// Flow:
// Ask remote for the device name. If it's set, emit it.
// If not, emit the default name.
//

DeviceNames::DeviceNames(const QString& userEmail):
    AttributeRequest(userEmail),
    mNameSuffix(0),
    mDeviceName(getDefaultDeviceName()),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    mega::MegaApi::log(
        mega::MegaApi::LOG_LEVEL_DEBUG,
        QString::fromUtf8("Default device name: \"%1\"").arg(mDeviceName).toUtf8().constData());
}

std::shared_ptr<DeviceNames> DeviceNames::requestDeviceName()
{
    return UserAttributesManager::instance().requestAttribute<DeviceNames>();
}

void DeviceNames::onRequestFinish(mega::MegaApi*,
                                  mega::MegaRequest* request,
                                  mega::MegaError* error)
{
    if (request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES)
    {
        switch (request->getType())
        {
            case mega::MegaRequest::TYPE_GET_ATTR_USER:
            {
                processGetDeviceNameCallback(request, error);
                break;
            }
            case mega::MegaRequest::TYPE_SET_ATTR_USER:
            {
                processSetDeviceNameCallback(request, error);
                break;
            }
        }
    }
}

void DeviceNames::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_DEVICE_NAMES);
}

AttributeRequest::RequestInfo DeviceNames::fillRequestInfo()
{
    std::function<void()> requestFunc = []()
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Requesting device name");
        MegaSyncApp->getMegaApi()->getUserAttribute(static_cast<char*>(nullptr),
                                                    mega::MegaApi::USER_ATTR_DEVICE_NAMES,
                                                    nullptr);
    };

    QSharedPointer<ParamInfo> paramInfo(new ParamInfo(
        requestFunc,
        QList<int>() << mega::MegaError::API_OK
                     << mega::MegaError::API_ENOENT // Case we have to set it
                     << mega::MegaError::API_EEXIST)); // Case where the name is already taken
    ParamInfoMap paramInfoMap({{mega::MegaApi::USER_ATTR_DEVICE_NAMES, paramInfo}});
    RequestInfo ret(paramInfoMap,
                    QMap<uint64_t, int>({{mega::MegaUser::CHANGE_TYPE_DEVICE_NAMES,
                                          mega::MegaApi::USER_ATTR_DEVICE_NAMES}}));
    return ret;
}

bool DeviceNames::isAttributeReady() const
{
    return !isRequestPending();
}

QString DeviceNames::getDeviceName() const
{
    return mDeviceName;
}

QString DeviceNames::getDeviceName(const DeviceId& deviceId) const
{
    return mAccountDeviceNames.value(deviceId);
}

QString DeviceNames::getDefaultDeviceName()
{
    const auto MAX_DEVICE_NAME_SIZE = 28;

    QString deviceName = Platform::getInstance()->getDeviceName();
    if (deviceName.length() > MAX_DEVICE_NAME_SIZE)
    {
        deviceName.truncate(MAX_DEVICE_NAME_SIZE);
    }

    // If empty, use generic one.
    return deviceName.isEmpty() ?
               QCoreApplication::translate("UserAttributes::DeviceName", "My computer") :
               deviceName;
}

bool DeviceNames::setDeviceName(const QString& deviceName)
{
    if (mDeviceName != deviceName)
    {
        mDeviceName = deviceName;

        setDeviceNameAttribute();
    }
    else
    {
        emit attributeReady(mDeviceName);
    }

    return true;
}

void DeviceNames::setDeviceNameAttribute()
{
    mega::MegaApi::log(
        mega::MegaApi::LOG_LEVEL_INFO,
        QString::fromUtf8("Setting Device name to \"%1\"").arg(mDeviceName).toUtf8().constData());

    mRequestInfo.mParamInfo[mega::MegaApi::USER_ATTR_DEVICE_NAMES]->setPending(true);
    MegaSyncApp->getMegaApi()->setDeviceName(nullptr, mDeviceName.toUtf8().constData());
}

QMap<DeviceNames::DeviceId, DeviceNames::Name> DeviceNames::getDeviceNames() const
{
    return mAccountDeviceNames;
}

void DeviceNames::processGetDeviceNameCallback(mega::MegaRequest* request, mega::MegaError* error)
{
    if (error->getErrorCode() == mega::MegaError::API_OK)
    {
        mAccountDeviceNames.clear();
        mega::MegaStringMap* deviceNameMap = request->getMegaStringMap();
        std::unique_ptr<mega::MegaStringList> deviceNameKeys(deviceNameMap->getKeys());

        for (int keyIndex = 0; keyIndex < deviceNameKeys->size(); ++keyIndex)
        {
            QString deviceId = QString::fromUtf8(deviceNameKeys->get(keyIndex));
            QString deviceName = QString::fromUtf8(
                QByteArray::fromBase64(deviceNameMap->get(deviceNameKeys->get(keyIndex))));

            mAccountDeviceNames.insert(deviceId, deviceName);
        }

        if (mAccountDeviceNames.contains(QString::fromUtf8(mMegaApi->getDeviceId())))
        {
            mDeviceName = mAccountDeviceNames[QString::fromUtf8(mMegaApi->getDeviceId())];

            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                               QString::fromUtf8("Got device name from remote: \"%1\"")
                                   .arg(mDeviceName)
                                   .toUtf8()
                                   .constData());
        }

        emit attributeReady(mDeviceName);
    }
    else if (error->getErrorCode() == mega::MegaError::API_ENOENT)
    {
        setDeviceNameAttribute();
    }
    else
    {
        QString errorMsg = QString::fromUtf8(error->getErrorString());
        QString logMsg(
            QString::fromUtf8("Error getting device name from remote: \"%1\"").arg(errorMsg));
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
    }
}

void DeviceNames::processSetDeviceNameCallback(mega::MegaRequest* request, mega::MegaError* error)
{
    if (error->getErrorCode() == mega::MegaError::API_OK)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                           QString::fromUtf8("Device name(%1) successfully set on remote")
                               .arg(mDeviceName)
                               .toUtf8()
                               .constData());
        mNameSuffix = 0;
        mDeviceName = QString::fromUtf8(request->getName());
        emit attributeReady(mDeviceName);
    }
    else
    {
        QString logMsg(QString::fromUtf8("Error setting device name on remote: \"%1\"")
                           .arg(QString::fromUtf8(error->getErrorString())));
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

        if (error->getErrorCode() == mega::MegaError::API_EEXIST)
        {
            // Increment suffix and retry
            mNameSuffix++;
            mDeviceName = mDeviceName + QString::fromLatin1(" - ") + QString::number(mNameSuffix);
            setDeviceNameAttribute();
        }
        else
        {
            mDeviceName.clear();
        }
    }
}

}
