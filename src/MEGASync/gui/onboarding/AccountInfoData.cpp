#include "AccountInfoData.h"
#include "MegaApplication.h"

using namespace mega;

const long long AccountInfoData::INITIAL_SPACE = 1000000;

AccountInfoData::AccountInfoData(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new QTMegaRequestListener(mMegaApi, this))
    , mType(AccountType::ACCOUNT_TYPE_NOT_SET)
    , mTotalStorage(QString())
    , mUsedStorage(QString())
    , mNewUser(false)
{
    requestAccountInfoData();
}

void AccountInfoData::requestAccountInfoData()
{
    mMegaApi->getAccountDetails(this->mDelegateListener.get());
}

void AccountInfoData::aboutToBeDestroyed()
{
    mDelegateListener = nullptr;
}

void AccountInfoData::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_ACCOUNT_DETAILS:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "AccountInfoData::onRequestFinish -> TYPE_ACCOUNT_DETAILS API_OK";
                MegaAccountDetails* accountDetails = request->getMegaAccountDetails();
                mType = static_cast<AccountInfoData::AccountType>(accountDetails->getProLevel());
                mTotalStorage = Utilities::getSizeString(accountDetails->getStorageMax());
                mUsedStorage = Utilities::getSizeString(accountDetails->getStorageUsed());
                mNewUser = accountDetails->getStorageUsed() < INITIAL_SPACE
                            && Preferences::instance()->cloudDriveFiles() <= 1
                            && SyncInfo::instance()->getNumSyncedFolders(SyncInfo::AllHandledSyncTypes) == 0;
                emit accountDetailsChanged();
            } else {
                qDebug() << "AccountInfoData::onRequestFinish -> TYPE_ACCOUNT_DETAILS Error code -> "
                         << error->getErrorCode();
            }
        }
    }
}
