#include "UserAttributesManager.h"

#include "megaapi.h"
#include "mega/types.h"

namespace UserAttributes
{
UserAttributesManager::UserAttributesManager()
{
    const auto megaApp = static_cast<MegaApplication*>(qApp);
    mDelegateListener = new mega::QTMegaListener(megaApp->getMegaApi(), this);
    megaApp->getMegaApi()->addListener(mDelegateListener);
}

void UserAttributesManager::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *incoming_request, mega::MegaError *e)
{
    if(incoming_request->getType() == mega::MegaRequest::TYPE_GET_ATTR_USER)
    {
        auto userEmail = QString::fromUtf8(incoming_request->getEmail());
        foreach(auto request, mRequests.values(userEmail))
        {
            request->onRequestFinish(api, incoming_request, e);
        }
    }
}

void UserAttributesManager::onUsersUpdate(mega::MegaApi*, mega::MegaUserList *users)
{
    for (int i = 0; i < users->size(); i++)
    {
        mega::MegaUser *user = users->get(i);
        auto userEmail = QString::fromUtf8(user->getEmail());
        foreach(auto request, mRequests.values(userEmail))
        {
            request->updateAttributes(user);
        }
    }
}
}
