#ifndef USERATTRIBUTESMANAGER_H
#define USERATTRIBUTESMANAGER_H

#include <QObject>
#include <QMultiMap>

#include <QTMegaListener.h>

#include <memory>

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

    void reset();

    template <typename AttributeClass>
    std::shared_ptr<AttributeClass> requestAttribute(const char* user_email = nullptr)
    {
        QString userEmail = QString::fromUtf8(user_email);
        QString mapKey = getKey(userEmail);

        auto classType = typeid(AttributeClass).name();

        auto userRequests = mRequests.values(mapKey);
        foreach(auto& request, userRequests)
        {
            auto requestType = typeid(*request).name();
            if(requestType == classType)
            {
                //Do not request attribute again, as it is done the first time
                return std::dynamic_pointer_cast<AttributeClass>(request);
            }
        }

        auto request = std::make_shared<AttributeClass>(userEmail);
        mRequests.insert(mapKey, std::static_pointer_cast<AttributeRequest>(request));
        request->requestAttribute();

        return request;
    }

    void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void onUsersUpdate(mega::MegaApi *, mega::MegaUserList *users) override;

private:
    explicit UserAttributesManager();
    QString getKey(const QString& userEmail) const;

    std::unique_ptr<mega::QTMegaListener> mDelegateListener;
    QMultiMap<QString, std::shared_ptr<AttributeRequest>> mRequests;
};
}

#endif // USERATTRIBUTESMANAGER_H
