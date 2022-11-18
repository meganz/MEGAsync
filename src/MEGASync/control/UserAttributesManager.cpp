#include "UserAttributesManager.h"

#include "MegaApplication.h"
#include "Utilities.h"

#include "megaapi.h"
#include "mega/types.h"
#include <assert.h>


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
    if(incoming_request->getType() == mega::MegaRequest::TYPE_GET_ATTR_USER)
    {
        auto userEmail = QString::fromUtf8(incoming_request->getEmail());
        foreach(auto request, mRequests.values(userEmail))
        {
            if(request->getRequestInfo().mParamInfo.contains(incoming_request->getParamType()))
            {
                auto paramInfo = request->getRequestInfo().mParamInfo.value(incoming_request->getParamType());
                paramInfo->setNeedsRetry(e->getErrorCode());
                paramInfo->setPending(false);
                request->onRequestFinish(api, incoming_request, e);
            }
        }

        if(e && e->getErrorCode() != mega::MegaError::API_OK)
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error requesting user attribute. User: %1 Attribute: %2 Error: %3").arg(userEmail).arg(incoming_request->getParamType()).arg(e->getErrorCode()).toUtf8().constData());
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
            if(user->isOwnChange() <= 0)
            {
                auto userEmail = QString::fromUtf8(user->getEmail());
                foreach(auto request, mRequests.values(userEmail))
                {
                    foreach(auto changeType, request->getRequestInfo().mChangedTypes.keys())
                    {
                        if(user->getChanges() & changeType)
                        {
                            auto paramType = request->getRequestInfo().mChangedTypes.value(changeType, -1);
                            if(paramType >= 0)
                            {
                                request->getRequestInfo().mParamInfo.value(paramType)->mNeedsRetry = true;
                                request->requestUserAttribute(paramType);
                            }
                        }
                    }
                }
            }
        }
    }
}

void AttributeRequest::RequestInfo::ParamInfo::setNeedsRetry(int errCode)
{
    mNeedsRetry = !mNoRetryErrCodes.isEmpty() &&!mNoRetryErrCodes.contains(errCode);
}

void AttributeRequest::RequestInfo::ParamInfo::setPending(bool isPending)
{
    mIsPending = isPending;
}

bool AttributeRequest::isAttributeRequestPending(int attribute) const
{
    if(getRequestInfo().mParamInfo.contains(attribute))
    {
        return getRequestInfo().mParamInfo.value(attribute)->mIsPending;
    }
    assert("Malformed map, no attribute found");
    return false;
}

bool AttributeRequest::isRequestPending() const
{
    bool isPending = false;
    auto paramInfoIt (getRequestInfo().mParamInfo.cbegin());
    while (!isPending && paramInfoIt != getRequestInfo().mParamInfo.cend())
    {
        isPending |= paramInfoIt.value()->mIsPending;
        paramInfoIt++;
    }
    return isPending;
}


bool AttributeRequest::attributeRequestNeedsRetry(int attribute) const
{
    if(getRequestInfo().mParamInfo.contains(attribute))
    {
        return getRequestInfo().mParamInfo.value(attribute)->mNeedsRetry
                && !getRequestInfo().mParamInfo.value(attribute)->mIsPending;
    }
    assert("Malformed map, no attribute found");
    return false;
}

void AttributeRequest::requestUserAttribute(int attribute)
{
    if(attributeRequestNeedsRetry(attribute))
    {
        auto val = getRequestInfo().mParamInfo.value(attribute);
        val->setPending(true);
        val->requestFunc();
    }
}

}//end namespace UserAttributes
