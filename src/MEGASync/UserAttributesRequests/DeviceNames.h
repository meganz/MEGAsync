#ifndef DEVICE_NAME_ATTRIBUTES_REQUESTS_H
#define DEVICE_NAME_ATTRIBUTES_REQUESTS_H

#include "UserAttributesManager.h"

namespace UserAttributes
{
class DeviceNames: public AttributeRequest
{
    Q_OBJECT

public:
    using DeviceId = QString;
    using Name = QString;

    DeviceNames(const QString& userEmail);

    static std::shared_ptr<DeviceNames> requestDeviceNames();
    static Name getDefaultDeviceName();

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest* incoming_request,
                         mega::MegaError* e) override;
    void requestAttribute() override;
    RequestInfo fillRequestInfo() override;

    bool isAttributeReady() const override;

    Name getDeviceName() const;
    Name getDeviceName(const DeviceId& deviceId) const;
    void setDeviceName(const Name& newDeviceName);
    void setDeviceNameAttribute();
    QMap<DeviceId, Name> getDeviceNames() const;

signals:
    void attributeReady(const Name& deviceName);

private:
    void processGetDeviceNamesCallback(mega::MegaRequest* incoming_request, mega::MegaError* e);
    void processSetDeviceNameCallback(mega::MegaRequest* incoming_request, mega::MegaError* e);
    void updateAutoDeviceName();

    int mAutoDeviceNameSuffix;
    Name mDeviceName;
    mega::MegaApi* mMegaApi = nullptr;
    QMap<DeviceId, Name> mAccountDeviceNames;
};
}

#endif // DEVICE_NAME_ATTRIBUTES_REQUESTS_H
