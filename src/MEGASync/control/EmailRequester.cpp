#include "EmailRequester.h"

#include "MegaApplication.h"

#include "mega/types.h"

#include <QMutexLocker>

EmailRequester* EmailRequester::mInstance = nullptr;

EmailRequester::EmailRequester():
    mMegaApi(MegaSyncApp->getMegaApi()),
    mGlobalListener(mega::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
{
    MegaSyncApp->getMegaApi()->addGlobalListener(mGlobalListener.get());
}

EmailRequester::~EmailRequester()
{
    mRequestsData.clear();
}

void EmailRequester::addEmailTracking(mega::MegaHandle userHandle, const QString& email)
{
    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt == mRequestsData.end())
    {
        RequestInfo requestInfo;
        requestInfo.requestFinished = true;
        requestInfo.email = email;
        mRequestsData[userHandle] = requestInfo;
    }
}

EmailRequester* EmailRequester::instance()
{
    if (mInstance == nullptr)
    {
        mInstance = new EmailRequester();
    }

    return mInstance;
}

void EmailRequester::onUsersUpdate(mega::MegaApi* api, mega::MegaUserList* users)
{
    Q_UNUSED(api);

    /*
     * Look for alerts from users that changed their email and update them
     */
    bool dataChanged = false;
    {
        QMutexLocker locker(&mRequestsDataLock);

        if (mRequestsData.isEmpty())
        {
            return;
        }

        for(auto userIndex = 0; userIndex < users->size(); ++userIndex)
        {
            auto user = users->get(userIndex);

            auto requestDataIt = mRequestsData.find(user->getHandle());
            if (requestDataIt != mRequestsData.end())
            {
                auto email = QString::fromUtf8(user->getEmail());
                if (requestDataIt->email != email)
                {
                    requestDataIt->email = email;
                    dataChanged = true;
                }
            }
        }
    }

    if (dataChanged)
    {
        emit emailChanged();
    }
}

QString EmailRequester::getEmail(mega::MegaHandle userHandle, bool forceRequest)
{
    QString email;

    {
        QMutexLocker locker(&mRequestsDataLock);
        auto foundUserHandleIt = mRequestsData.find(userHandle);
        if (foundUserHandleIt != mRequestsData.end())
        {
            if (foundUserHandleIt->requestFinished)
            {
                email = foundUserHandleIt->email;
            }
        }
    }

    if (email.isEmpty() && forceRequest)
    {
        requestEmail(userHandle);
    }

    return email;
}

void EmailRequester::requestEmail(mega::MegaHandle userHandle)
{
    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt == mRequestsData.end())
    {
        RequestInfo requestInfo;
        requestInfo.requestFinished = false;
        mRequestsData[userHandle] = requestInfo;

        mMegaApi->getUserEmail(userHandle, new mega::OnFinishOneShot(mMegaApi,  this, [this, userHandle]
            (bool isContextValid, const mega::MegaRequest& request, const mega::MegaError& error)
            {
                if(request.getType() == mega::MegaRequest::TYPE_GET_USER_EMAIL && error.getErrorCode() == mega::MegaError::API_OK)
                {
                    if (request.getEmail() != nullptr && isContextValid)
                    {
                        QMutexLocker locker(&mRequestsDataLock);

                        auto foundUserHandleIt = mRequestsData.find(userHandle);
                        if (foundUserHandleIt != mRequestsData.end())
                        {
                            foundUserHandleIt->requestFinished = true;
                            foundUserHandleIt->email = QString::fromUtf8(request.getEmail());

                            emit emailChanged();
                        }
                    }
                }
            })
        );
    }
}

