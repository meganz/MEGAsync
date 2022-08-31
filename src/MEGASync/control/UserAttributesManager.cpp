#include "UserAttributesManager.h"

#include "megaapi.h"
#include "mega/types.h"

namespace UserAttributes
{

UserAttributesManager::UserAttributesManager() :
    mDelegateListener(new mega::QTMegaListener(MegaSyncApp->getMegaApi(), this))
{
    MegaSyncApp->getMegaApi()->addListener(mDelegateListener.get());
}

void UserAttributesManager::reset()
{
    mRequests.clear();
}

void UserAttributesManager::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *incoming_request, mega::MegaError *e)
{
    auto reqType (incoming_request->getType());
    if(reqType == mega::MegaRequest::TYPE_GET_ATTR_USER
            || reqType == mega::MegaRequest::TYPE_SET_ATTR_USER)
    {
        auto userEmail = QString::fromUtf8(incoming_request->getEmail());

        // Forward to requests related to the corresponding user
        foreach(auto request, mRequests.values(getKey(userEmail)))
        {
            request->onRequestFinish(api, incoming_request, e);
        }
    }
}

void UserAttributesManager::onUsersUpdate(mega::MegaApi*, mega::MegaUserList *users)
{
    if (users)
    {
        for (int i = 0; i < users->size(); i++)
        {
            mega::MegaUser *user = users->get(i);
            if(!user->isOwnChange())
            {
                auto userEmail = QString::fromUtf8(user->getEmail());
                foreach(auto request, mRequests.values(getKey(userEmail)))
                {
                    request->updateAttributes(user);
                }
            }
        }
    }
}

QString UserAttributesManager::getKey(const QString& userEmail) const
{
    static QString loggedUserKey(QLatin1Char('u'));

    QString key = loggedUserKey;
    // If the email is not empty, use key 'u' for current user.
    if (!userEmail.isEmpty())
    {
        std::unique_ptr<char[]> currentUserEmail (MegaSyncApp->getMegaApi()->getMyEmail());
        if (userEmail != QString::fromUtf8(currentUserEmail.get()))
        {
            key = userEmail;
        }
    }

    return key;
}


}//end namespace UserAttributes
