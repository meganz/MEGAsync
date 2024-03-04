#include "AccountInfoData.h"
#include "MegaApplication.h"

using namespace mega;

static const long long MIN_USED_STORAGE_THRESHOLD = 1000000;

AccountInfoData::AccountInfoData(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new QTMegaRequestListener(mMegaApi, this))
    , mGlobalListener(new QTMegaGlobalListener(mMegaApi, this))
    , mType(AccountType::ACCOUNT_TYPE_NOT_SET)
    , mTotalStorage()
    , mUsedStorage()
    , mBelowMinUsedStorageThreshold(false)
    , mInitialized(false)
{
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

AccountInfoData* AccountInfoData::instance(QQmlEngine* qmlEngine, QJSEngine*)
{
    AccountInfoData* accountInfoData = new AccountInfoData(qmlEngine);

    return accountInfoData;
}

void AccountInfoData::requestAccountInfoData()
{
    if (!mInitialized) {
        mMegaApi->getAccountDetails(mDelegateListener.get());
    }
}

void AccountInfoData::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_ACCOUNT_DETAILS:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                MegaAccountDetails* accountDetails = request->getMegaAccountDetails();
                mType = static_cast<AccountInfoData::AccountType>(accountDetails->getProLevel());
                mTotalStorage = Utilities::getSizeString(accountDetails->getStorageMax());
                mUsedStorage = Utilities::getSizeString(accountDetails->getStorageUsed());
                mBelowMinUsedStorageThreshold = accountDetails->getStorageUsed() < MIN_USED_STORAGE_THRESHOLD;

                mInitialized = true;

                emit accountDetailsChanged();
                emit usedStorageChanged();
            }
            else
            {
                qDebug() << "AccountInfoData::onRequestFinish -> TYPE_ACCOUNT_DETAILS Error code -> "
                         << error->getErrorCode();
            }
            break;
        }
    }
}

void AccountInfoData::onEvent(MegaApi*, MegaEvent* event)
{
    switch(event->getType())
    {
        case MegaEvent::EVENT_STORAGE_SUM_CHANGED:
        {
            long long usedStorage = event->getNumber();
            mUsedStorage = Utilities::getSizeString(usedStorage);
            mBelowMinUsedStorageThreshold = usedStorage < MIN_USED_STORAGE_THRESHOLD;

            emit accountDetailsChanged();
            emit usedStorageChanged();
            break;
        }
    }
}

void AccountInfoData::onAccountUpdate(mega::MegaApi*)
{
    mInitialized = false;

    requestAccountInfoData();
}
