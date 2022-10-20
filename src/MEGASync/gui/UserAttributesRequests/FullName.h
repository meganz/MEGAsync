#ifndef FULLNAMEATTRIBUTESREQUESTS_H
#define FULLNAMEATTRIBUTESREQUESTS_H

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class FullName : public AttributeRequest
{
    Q_OBJECT

public:
    FullName(const QString& userEmail) : AttributeRequest(userEmail){}

    static std::shared_ptr<const FullName> requestFullName(const char* user_email);

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    RequestInfo fillRequestInfo() override;

    QString getFullName() const;    
    //In order to use in Rich Text labels (otherwise some characters may be interpreted as HMTL)
    QString getRichFullName() const;

    bool isAttributeReady() const override;

    const QString& getFirstName() const;
    const QString& getLastName() const;

signals:
    //In order to use in Rich Text labels (otherwise some characters may be interpreted as HMTL)
    void attributeReadyRichText(const QString&);
    void attributeReady(const QString&);

private:
    QString mFirstName;
    QString mLastName;
};
}

#endif // USERATTRIBUTESREQUESTS_H
