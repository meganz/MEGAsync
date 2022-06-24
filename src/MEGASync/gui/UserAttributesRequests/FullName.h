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
    void updateAttributes(mega::MegaUser* user) override;

    QString getFullName() const;
    bool isAttributeReady() const override;
    const QString& getFirstName() const;
    const QString& getLastName() const;

signals:
    void attributeReady(const QString&);

private:
    void requestFirstNameAttribute();
    void requestLastNameAttribute();

    QString mFirstName;
    QString mLastName;
};
}

#endif // USERATTRIBUTESREQUESTS_H
