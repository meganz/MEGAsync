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

    static std::shared_ptr<DeviceNames> requestDeviceName();

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest* incoming_request,
                         mega::MegaError* e) override;
    void requestAttribute() override;
    RequestInfo fillRequestInfo() override;

    bool isAttributeReady() const override;

    QString getDeviceName() const;
    QString getDefaultDeviceName();
    bool setDeviceName(const QString& deviceName);
    void setDeviceNameAttribute();
    QMap<DeviceId, Name> getDeviceNames();

signals:
    void attributeReady(const QString& deviceName);

private:
    void processGetDeviceNameCallback(mega::MegaRequest* incoming_request, mega::MegaError* e);
    void processSetDeviceNameCallback(mega::MegaRequest* incoming_request, mega::MegaError* e);

    int mNameSuffix;
    QString mDeviceName;
    mega::MegaApi* mMegaApi = nullptr;
    QMap<DeviceId, Name> mAccountDeviceNames;
};
}

#endif // DEVICE_NAME_ATTRIBUTES_REQUESTS_H
