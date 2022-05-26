#ifndef USERATTRIBUTESMANAGER_H
#define USERATTRIBUTESMANAGER_H

#include <QObject>

#include <QTMegaListener.h>
#include <MegaApplication.h>

namespace UserAttributes
{
class AttributeRequest : public QObject
{
    Q_OBJECT

public:
    AttributeRequest(const QString& userEmail):mUserEmail(userEmail){}

    virtual void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e) = 0;
    virtual void requestAttribute()= 0;
    virtual void updateAttributes(mega::MegaUser* user) = 0;
    virtual bool isAttributeReady() const = 0;

    const QString& getEmail() const {return mUserEmail;}

protected:
    QString mUserEmail;
};

class UserAttributesManager : public mega::MegaListener
{
public:
    static UserAttributesManager& instance()
    {
        static UserAttributesManager    instance;
        return instance;
    }

    template <typename AttributeClass>
    std::shared_ptr<AttributeClass> requestAttribute(const char* user_email)
    {
        QString userEmail = QString::fromUtf8(user_email);

        auto classType = typeid(AttributeClass).name();

        auto requestsByEmail = mRequests.values(userEmail);
        foreach(auto& request, requestsByEmail)
        {
            auto requestType = typeid(*request).name();
            if(requestType == classType)
            {
                return std::dynamic_pointer_cast<AttributeClass>(request);
            }
        }

        auto request = std::make_shared<AttributeClass>(userEmail);
        mRequests.insertMulti(userEmail, std::static_pointer_cast<AttributeRequest>(request));
        request->requestAttribute();

        return request;
    }

    void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void onUsersUpdate(mega::MegaApi *, mega::MegaUserList *users) override;

private:
    explicit UserAttributesManager();

    std::unique_ptr<mega::QTMegaListener> mDelegateListener;
    QMap<QString, std::shared_ptr<AttributeRequest>> mRequests;
};
}

#endif // USERATTRIBUTESMANAGER_H
