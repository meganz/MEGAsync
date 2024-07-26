#include "EmailRequester.h"
#include "MegaApplication.h"

#include "mega/types.h"
#include "RequestListenerManager.h"
#include <QMutexLocker>

EmailRequester* EmailRequester::mInstance = nullptr;

EmailRequester::EmailRequester():
    mMegaApi(MegaSyncApp->getMegaApi()),
    mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
{
    MegaSyncApp->getMegaApi()->addGlobalListener(mGlobalListener.get());
}

void EmailRequester::reset()
{
    qDeleteAll(mRequestsData);
    mRequestsData.clear();
}

RequestInfo* EmailRequester::addUser(mega::MegaHandle userHandle, const QString& email)
{
    assert(userHandle != mega::INVALID_HANDLE);

    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt == mRequestsData.end())
    {
        auto requestInfo = new RequestInfo(this);
        requestInfo->setEmail(email);

        mRequestsData[userHandle] = requestInfo;

        if (email.isEmpty())
        {
            requestEmail(userHandle);
        }
    }

    return mRequestsData[userHandle];
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
                if ((*requestDataIt)->getEmail() != email)
                {
                    (*requestDataIt)->setEmail(email);
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
            if ((*foundUserHandleIt)->requestFinished)
            {
                email = (*foundUserHandleIt)->getEmail();
            }
        }
    }

    return email;
}

RequestInfo* EmailRequester::getRequest(mega::MegaHandle userHandle, const QString& email)
{
    return instance()->addUser(userHandle, email);
}

void EmailRequester::requestEmail(mega::MegaHandle userHandle)
{
    QMutexLocker locker(&mRequestsDataLock);

    auto foundUserHandleIt = mRequestsData.find(userHandle);
    if (foundUserHandleIt != mRequestsData.end())
    {
        if(!(*foundUserHandleIt)->requestFinished)
        {
            return;
        }

        (*foundUserHandleIt)->requestFinished = false;

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [this](mega::MegaRequest* request, mega::MegaError* e) {
            if (request->getType() == mega::MegaRequest::TYPE_GET_USER_EMAIL)
            {
                QString email;

                if (e->getErrorCode() == mega::MegaError::API_OK && request->getEmail() != nullptr)
                {
                    email = QString::fromUtf8(request->getEmail());
                }

                QMutexLocker locker(&mRequestsDataLock);

                auto foundUserHandleIt = mRequestsData.find(request->getNodeHandle());
                if (foundUserHandleIt != mRequestsData.end())
                {
                    (*foundUserHandleIt)->requestFinished = true;
                    (*foundUserHandleIt)->setEmail(email);
                }
            }
        });

        mMegaApi->getUserEmail(userHandle, listener.get());
    }
}


void RequestInfo::setEmail(const QString& email)
{
    if(!email.isEmpty() && mEmail != email)
    {
        mEmail = email;
        emit emailChanged(mEmail);
    }
}

QString RequestInfo::getEmail() const
{
    return mEmail;
}
