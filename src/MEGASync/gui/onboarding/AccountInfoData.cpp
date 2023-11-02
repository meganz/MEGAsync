#include "AccountInfoData.h"
#include "MegaApplication.h"

using namespace mega;

const long long AccountInfoData::MIN_THRESHOLD = 1000000;

AccountInfoData::AccountInfoData(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new QTMegaRequestListener(mMegaApi, this))
    , mGlobalListener(new QTMegaGlobalListener(mMegaApi, this))
    , mType(AccountType::ACCOUNT_TYPE_NOT_SET)
    , mTotalStorage()
    , mUsedStorage()
    , mBelowMinThreshold(false)
    , mInitialized(false)
{
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

AccountInfoData* AccountInfoData::instance(QQmlEngine* qmlEngine, QJSEngine*)
{
    static AccountInfoData accountInfoData(qmlEngine);

    return &accountInfoData;
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
                mBelowMinThreshold = accountDetails->getStorageUsed() < MIN_THRESHOLD;

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
            auto usedStorage = event->getNumber();
            mUsedStorage = Utilities::getSizeString(usedStorage);
            mBelowMinThreshold = usedStorage < MIN_THRESHOLD;

            emit accountDetailsChanged();
            emit usedStorageChanged();
        }
    }
}

void AccountInfoData::onAccountUpdate(mega::MegaApi*)
{
    requestAccountInfoData();
}
