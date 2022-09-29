#ifndef DEVICENAMEATTRIBUTESREQUESTS_H
#define DEVICENAMEATTRIBUTESREQUESTS_H

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class DeviceName : public AttributeRequest
{
    Q_OBJECT

public:
    DeviceName(const QString& userEmail);

    static std::shared_ptr<DeviceName> requestDeviceName();

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    void updateAttributes(mega::MegaUser* user) override;
    bool isAttributeReady() const override;

    QString getDeviceName();
    QString getDefaultDeviceName();

signals:
    void attributeReady(const QString&);

private:
    void requestDeviceNameAttribute(bool forceRefresh);
    void setDeviceNameAttribute();

    bool mIsDeviceNameSetOnRemote;
    bool mIsRequestFinished;
    QString mDeviceName;
    int  mNameSuffix;
};
}

#endif // DEVICENAMEATTRIBUTESREQUESTS_H
