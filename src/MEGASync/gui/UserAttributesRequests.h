#ifndef USERATTRIBUTESREQUESTS_H
#define USERATTRIBUTESREQUESTS_H

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class FullNameAttributeRequest : public AttributeRequest
{
    Q_OBJECT

public:
    FullNameAttributeRequest(const QString& userEmail) : AttributeRequest(userEmail){}

    static std::shared_ptr<FullNameAttributeRequest> requestFullName(const char* user_email);

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *) override;
    void requestAttribute() override;
    void updateAttributes(mega::MegaUser* user) override;

    QString getFullName();

private:
    QString mFirstName;
    QString mLastName;
};

class AvatarAttributeRequest : public AttributeRequest
{
    Q_OBJECT

public:
    AvatarAttributeRequest(const QString& userEmail) : AttributeRequest(userEmail){}

    static std::shared_ptr<AvatarAttributeRequest> requestAvatar(const char* user_email);

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *) override;
    void requestAttribute() override;
    void updateAttributes(mega::MegaUser* user) override;

    QPixmap GetPixmap(int diameter);

private:
    QString mFilePath;
};
}

#endif // USERATTRIBUTESREQUESTS_H
