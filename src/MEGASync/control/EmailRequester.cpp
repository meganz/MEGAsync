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

void EmailRequester::reset()
{
    mRequestsData.clear();
}

RequestInfo* EmailRequester::addUser(mega::MegaHandle userHandle, const QString& email)
{
    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt == mRequestsData.end())
    {
        auto requestInfo = std::make_shared<RequestInfo>();
        requestInfo->requestFinished = !email.isEmpty();
        requestInfo->email = email;

        mRequestsData[userHandle] = requestInfo;

        if (email.isEmpty())
        {
            requestEmail(userHandle);
        }
    }

    return mRequestsData[userHandle].get();
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

    // check function precondition
    if (users == nullptr)
    {
        return;
    }

    /*
     * Look for alerts from users that changed their email and update them
     */
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
                if (requestDataIt->get()->email != email)
                {
                    requestDataIt->get()->email = email;
                    emit requestDataIt->get()->emailChanged(email);
                }
            }
        }
    }
}

QString EmailRequester::getEmail(mega::MegaHandle userHandle)
{
    QString email;

    {
        QMutexLocker locker(&mRequestsDataLock);
        auto foundUserHandleIt = mRequestsData.find(userHandle);
        if (foundUserHandleIt != mRequestsData.end())
        {
            if (foundUserHandleIt->get()->requestFinished)
            {
                email = foundUserHandleIt->get()->email;
            }
        }
    }

    return email;
}

void EmailRequester::requestEmail(mega::MegaHandle userHandle)
{
    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt != mRequestsData.end())
    {
        mMegaApi->getUserEmail(userHandle, new mega::OnFinishOneShot(mMegaApi,  this, [this, userHandle]
            (bool isContextValid, const mega::MegaRequest& request, const mega::MegaError& error)
            {
                if(isContextValid && request.getType() == mega::MegaRequest::TYPE_GET_USER_EMAIL)
                {
                    QString email;

                    if (error.getErrorCode() == mega::MegaError::API_OK && request.getEmail() != nullptr)
                    {
                        email = QString::fromUtf8(request.getEmail());
                    }

                    QMutexLocker locker(&mRequestsDataLock);

                    auto foundUserHandleIt = mRequestsData.find(userHandle);
                    if (foundUserHandleIt != mRequestsData.end())
                    {
                        foundUserHandleIt->get()->requestFinished = true;
                        foundUserHandleIt->get()->email = email;
                        emit foundUserHandleIt->get()->emailChanged(email);
                    }
                }
            })
        );
    }
}

