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
    mAutoDeviceNameSuffix(0),
    mDeviceName(getDefaultDeviceName()),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    if (mMegaApi)
    {
        mDeviceId = QString::fromUtf8(mMegaApi->getDeviceId());
    }

    mega::MegaApi::log(
        mega::MegaApi::LOG_LEVEL_DEBUG,
        QString::fromUtf8("Default device name: \"%1\"").arg(mDeviceName).toUtf8().constData());
}

std::shared_ptr<DeviceNames> DeviceNames::requestDeviceNames()
{
    return UserAttributesManager::instance().requestAttribute<DeviceNames>();
}

DeviceNames::Name DeviceNames::getDefaultDeviceName()
{
    const auto MAX_DEVICE_NAME_SIZE = 28;

    auto deviceName = Platform::getInstance()->getDeviceName();
    deviceName.truncate(MAX_DEVICE_NAME_SIZE);

    // If empty, use generic one.
    return deviceName.isEmpty() ?
               QCoreApplication::translate("UserAttributes::DeviceName", "My computer") :
               deviceName;
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
                processGetDeviceNamesCallback(request, error);
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
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Requesting device names");
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

DeviceNames::Name DeviceNames::getDeviceName() const
{
    return mDeviceName;
}

DeviceNames::Name DeviceNames::getDeviceName(const DeviceId& deviceId) const
{
    return mAccountDeviceNames.value(deviceId);
}

void DeviceNames::setDeviceName(const DeviceNames::Name& newDeviceName)
{
    mDeviceName = newDeviceName;
    setDeviceNameAttribute();
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

void DeviceNames::processGetDeviceNamesCallback(mega::MegaRequest* request, mega::MegaError* error)
{
    if (error->getErrorCode() == mega::MegaError::API_OK)
    {
        mAccountDeviceNames.clear();
        mega::MegaStringMap* deviceNameMap = request->getMegaStringMap();
        std::unique_ptr<mega::MegaStringList> deviceNameKeys(deviceNameMap->getKeys());

        for (int keyIndex = 0; keyIndex < deviceNameKeys->size(); ++keyIndex)
        {
            const auto deviceId = QString::fromUtf8(deviceNameKeys->get(keyIndex));
            const auto deviceName = QString::fromUtf8(
                QByteArray::fromBase64(deviceNameMap->get(deviceNameKeys->get(keyIndex))));

            mAccountDeviceNames.insert(deviceId, deviceName);
        }

        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Got device names from remote");

        const auto curentDeviceName = mAccountDeviceNames.constFind(mDeviceId);
        if (curentDeviceName != mAccountDeviceNames.cend())
        {
            mDeviceName = curentDeviceName.value();

            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                               QString::fromUtf8("Current device name: \"%1\"")
                                   .arg(mDeviceName)
                                   .toUtf8()
                                   .constData());
            emit attributeReady(mDeviceName);
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                               "Current device name not set on remote");

            updateAutoDeviceName();
            setDeviceNameAttribute();
        }
    }
    else if (error->getErrorCode() == mega::MegaError::API_ENOENT)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Device names attribute does not exist");

        setDeviceNameAttribute();
    }
    else
    {
        const auto errorMsg = QString::fromUtf8(error->getErrorString());
        const auto logMsg =
            QString::fromUtf8("Error getting device names from remote: \"%1\"").arg(errorMsg);
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
    }
}

void DeviceNames::processSetDeviceNameCallback(mega::MegaRequest* request, mega::MegaError* error)
{
    if (error->getErrorCode() == mega::MegaError::API_OK)
    {
        mAutoDeviceNameSuffix = 0;
        mDeviceName = QString::fromUtf8(request->getName());

        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Device name successfully set on remote");

        emit attributeReady(mDeviceName);
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Error setting device name on remote");

        if (error->getErrorCode() == mega::MegaError::API_EEXIST)
        {
            updateAutoDeviceName();
            setDeviceNameAttribute();
        }
        else
        {
            mDeviceName.clear();
        }
    }
}

void DeviceNames::updateAutoDeviceName()
{
    // Make sure we don't use an already taken name
    const auto takenNames = mAccountDeviceNames.values();
    while (takenNames.contains(mDeviceName))
    {
        // Increment suffix
        mAutoDeviceNameSuffix++;
        mDeviceName = getDefaultDeviceName() + QString::fromLatin1(" - ") +
                      QString::number(mAutoDeviceNameSuffix);
    }
}
}
