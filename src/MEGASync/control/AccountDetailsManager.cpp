#include "AccountDetailsManager.h"

#include "Preferences.h"
#include "StatsEventHandler.h"
#include "DialogOpener.h"
#include "SettingsDialog.h"
#include "RequestListenerManager.h"

namespace
{
const int UserStatsStorageFlag = 0x01;
const int UserStatsTransferFlag = 0x02;
const int UserStatsProFlag = 0x04;
}

//
// Private UserStatsFlags definition.
//
AccountDetailsManager::UserStatsFlags::UserStatsFlags()
    : storage(false)
    , transfer(false)
    , pro(false)
{
}

AccountDetailsManager::UserStatsFlags::UserStatsFlags(bool storage,
                                                      bool transfer,
                                                      bool pro)
    : storage(storage)
    , transfer(transfer)
    , pro(pro)
{
}

void AccountDetailsManager::UserStatsFlags::parse(int flags)
{
    storage = flags & UserStatsStorageFlag;
    transfer = flags & UserStatsTransferFlag;
    pro = flags & UserStatsProFlag;
}

//
// Private UserStats definition.
//
template<typename Type>
void AccountDetailsManager::UserStats<Type>::updateWithValue(const UserStatsFlags &flags,
                                                             Type value)
{
    if (flags.storage)
    {
        mStorageValue = value;
    }
    if (flags.transfer)
    {
        mTransferValue = value;
    }
    if (flags.pro)
    {
        mProValue = value;
    }
}

template<typename Type>
void AccountDetailsManager::UserStats<Type>::updateWithValue(Type value)
{
    mStorageValue = value;
    mTransferValue = value;
    mProValue = value;
}

template<typename Type>
Type AccountDetailsManager::UserStats<Type>::storageValue() const
{
    return mStorageValue;
}

template<typename Type>
Type AccountDetailsManager::UserStats<Type>::transferValue() const
{
    return mTransferValue;
}

template<typename Type>
Type AccountDetailsManager::UserStats<Type>::proValue() const
{
    return mProValue;
}

//
// Public AccountDetailsManager definition.
//
AccountDetailsManager::AccountDetailsManager(mega::MegaApi* megaApi,
                                             QObject* parent)
    : QObject(parent)
    , mMegaApi(megaApi)
    , mPreferences(Preferences::instance())
{
    mDelegateListener = RequestListenerManager::instance().registerAndGetFinishListener(this);
    reset();
    mProExpirityTimer.setSingleShot(true);
    connect(&mProExpirityTimer, &QTimer::timeout, this, [this]()
    {
        updateUserStats(true, true, true, true, USERSTATS_PRO_EXPIRED);
    });
}

void AccountDetailsManager::reset()
{
    mInflightUserStats.updateWithValue(false);
    mLastRequestUserStats.updateWithValue(0);
    mQueuedUserStats.updateWithValue(false);
    mQueuedStorageUserStatsReason = 0;
}

void AccountDetailsManager::onRequestFinish(mega::MegaRequest* request,
                                            mega::MegaError* error)
{
    switch (request->getType())
    {
        case mega::MegaRequest::TYPE_ACCOUNT_DETAILS:
        {
            handleAccountDetailsReply(request, error);
            break;
        }
        default:
        {
            break;
        }
    }
}

void AccountDetailsManager::updateUserStats(bool storage, bool transfer, bool pro, bool force, int source)
{
    if (MegaSyncApp->finished())
    {
        return;
    }

    // If any are already pending, we don't need to fetch again.
    if (mInflightUserStats.storageValue())
    {
        storage = false;
    }
    if (mInflightUserStats.transferValue())
    {
        transfer = false;
    }
    if (mInflightUserStats.proValue())
    {
        pro = false;
    }

    if (!storage && !transfer && !pro)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                           "Skipped call to getSpecificAccountDetails()");
        return;
    }

    // If the oldest of the ones we want is too recent, skip (unless force).
    long long lastRequest = 0;
    if (storage && (!lastRequest || lastRequest > mLastRequestUserStats.storageValue()))
    {
        lastRequest = mLastRequestUserStats.storageValue();
    }
    if (transfer && (!lastRequest || lastRequest > mLastRequestUserStats.transferValue()))
    {
        lastRequest = mLastRequestUserStats.transferValue();
    }
    if (pro && (!lastRequest || lastRequest > mLastRequestUserStats.proValue()))
    {
        lastRequest = mLastRequestUserStats.proValue();
    }

    if (storage && source >= 0)
    {
        mQueuedStorageUserStatsReason |= (1 << source);
    }

    UserStatsFlags flags(storage, transfer, pro);
    long long lastRequestInterval = (QDateTime::currentMSecsSinceEpoch() - lastRequest);
    if (force || !lastRequest || lastRequestInterval > Preferences::MIN_UPDATE_STATS_INTERVAL)
    {
        mMegaApi->getSpecificAccountDetails(storage,
                                            transfer,
                                            pro,
                                            storage ? mQueuedStorageUserStatsReason : -1,
                                            mDelegateListener.get());
        if (flags.storage)
        {
            mQueuedStorageUserStatsReason = 0;
        }

        mInflightUserStats.updateWithValue(flags, true);
        mLastRequestUserStats.updateWithValue(flags, QDateTime::currentMSecsSinceEpoch());
    }
    else
    {
        mQueuedUserStats.updateWithValue(flags, true);
    }
}

void AccountDetailsManager::periodicUpdate()
{
    if (mQueuedUserStats.storageValue() || mQueuedUserStats.transferValue() || mQueuedUserStats.proValue())
    {
        bool storage = mQueuedUserStats.storageValue();
        bool transfer = mQueuedUserStats.transferValue();
        bool pro = mQueuedUserStats.proValue();
        mQueuedUserStats.updateWithValue(false);
        updateUserStats(storage, transfer, pro, false, -1);
    }
}

void AccountDetailsManager::handleAccountDetailsReply(mega::MegaRequest* request,
                                                      mega::MegaError* error)
{
    if(!canContinue(error))
    {
        return;
    }

    mFlags.parse(request->getNumDetails());
    mInflightUserStats.updateWithValue(mFlags, false);

    //Account details retrieved, update the preferences and the information dialog
    std::shared_ptr<mega::MegaAccountDetails> details(request->getMegaAccountDetails());

    MegaSyncApp->pushToThreadPool([=]()
    {
        std::shared_ptr<mega::MegaNodeList> inShares(mFlags.storage ? mMegaApi->getInShares() : nullptr);

        Utilities::queueFunctionInAppThread([=]()
        {
            processProFlag(details);
            processStorageFlag(details, inShares);
            processTransferFlag(details);

            emit accountDetailsUpdated();
        });
    });
}

void AccountDetailsManager::processProFlag(const std::shared_ptr<mega::MegaAccountDetails>& details)
{
    if (!mFlags.pro)
    {
        return;
    }

    mPreferences->setAccountType(details->getProLevel());
    if (details->getProLevel() != Preferences::ACCOUNT_TYPE_FREE)
    {
        if (details->getProExpiration()
                && mPreferences->proExpirityTime() != details->getProExpiration())
        {
            mPreferences->setProExpirityTime(details->getProExpiration());
            mProExpirityTimer.stop();
            const long long interval = qMax(0LL, details->getProExpiration() * 1000 - QDateTime::currentMSecsSinceEpoch());
            mProExpirityTimer.setInterval(static_cast<int>(interval));
            mProExpirityTimer.start();
        }
    }
    else
    {
        mPreferences->setProExpirityTime(0);
        mProExpirityTimer.stop();
    }

    notifyAccountObservers();
}

void AccountDetailsManager::processStorageFlag(const std::shared_ptr<mega::MegaAccountDetails>& details,
                                               const std::shared_ptr<mega::MegaNodeList>& inShares)
{
    if (!mFlags.storage)
    {
        return;
    }

    mPreferences->setTotalStorage(details->getStorageMax());
    MegaSyncApp->updateUsedStorage(true);

    // Set in preferences the storage, files and folder for the cloud drive, rubbish and vault.
    processNodesAndVersionsStorage(details);

    // Inshares
    processInShares(details, inShares);

    if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
    {
        // Update settings dialog if it exists, to show the correct versions size.
        dialog->getDialog()->storageChanged();
    }

    notifyStorageObservers();

    if (MegaSyncApp->getStorageOverquotaDialog())
    {
        MegaSyncApp->getStorageOverquotaDialog()->refreshStorageDetails();
    }
}

void AccountDetailsManager::processTransferFlag(const std::shared_ptr<mega::MegaAccountDetails>& details)
{
    if (!mFlags.transfer)
    {
        return;
    }

    const bool proUserIsOverquota(mMegaApi->getBandwidthOverquotaDelay() &&
                                  mPreferences->accountType() != Preferences::ACCOUNT_TYPE_FREE);
    if (proUserIsOverquota)
    {
        MegaSyncApp->getTransferQuota()->setOverQuota(std::chrono::seconds(mMegaApi->getBandwidthOverquotaDelay()));
    }

    mPreferences->setTotalBandwidth(details->getTransferMax());
    mPreferences->setBandwidthInterval(details->getTemporalBandwidthInterval());
    mPreferences->setUsedBandwidth(details->getTransferUsed());

    mPreferences->setTemporalBandwidthInterval(details->getTemporalBandwidthInterval());
    mPreferences->setTemporalBandwidth(details->getTemporalBandwidth());
    mPreferences->setTemporalBandwidthValid(details->isTemporalBandwidthValid());

    notifyBandwidthObservers();

    if (mPreferences->accountType() != Preferences::ACCOUNT_TYPE_FREE)
    {
        MegaSyncApp->getTransferQuota()->updateQuotaState();
    }
}

mega::MegaHandle AccountDetailsManager::processNode(const std::shared_ptr<mega::MegaNode>& node,
                                                    const std::shared_ptr<mega::MegaAccountDetails>& details,
                                                    std::function<void (mega::MegaHandle)> setStorageUsed,
                                                    std::function<void (mega::MegaHandle)> setNumFiles,
                                                    std::function<void (mega::MegaHandle)> setNumFolders)
{
    mega::MegaHandle handle = node ? node->getHandle() : mega::INVALID_HANDLE;
    setStorageUsed(details->getStorageUsed(handle));
    setNumFiles(details->getNumFiles(handle));
    setNumFolders(details->getNumFolders(handle));
    return handle;
}

void AccountDetailsManager::processNodesAndVersionsStorage(const std::shared_ptr<mega::MegaAccountDetails>& details)
{
    // Cloud Drive
    auto cloudDriveHandle = processNode(MegaSyncApp->getRootNode(),
                                        details,
                                        std::bind(&Preferences::setCloudDriveStorage, mPreferences, std::placeholders::_1),
                                        std::bind(&Preferences::setCloudDriveFiles, mPreferences, std::placeholders::_1),
                                        std::bind(&Preferences::setCloudDriveFolders, mPreferences, std::placeholders::_1));

    // Vault
    auto vaultHandle = processNode(MegaSyncApp->getVaultNode(),
                                   details,
                                   std::bind(&Preferences::setVaultStorage, mPreferences, std::placeholders::_1),
                                   std::bind(&Preferences::setVaultFiles, mPreferences, std::placeholders::_1),
                                   std::bind(&Preferences::setVaultFolders, mPreferences, std::placeholders::_1));

    // Rubbish
    auto rubbishHandle = processNode(MegaSyncApp->getRubbishNode(),
                                     details,
                                     std::bind(&Preferences::setRubbishStorage, mPreferences, std::placeholders::_1),
                                     std::bind(&Preferences::setRubbishFiles, mPreferences, std::placeholders::_1),
                                     std::bind(&Preferences::setRubbishFolders, mPreferences, std::placeholders::_1));

    // For versions, match the webclient by only counting the user's own nodes.
    // Versions in inshares are not cleared by 'clear versions'.
    // Also the no-parameter getVersionStorageUsed() double counts the versions in outshares.
    // Inshare storage count should include versions.
    mPreferences->setVersionsStorage(details->getVersionStorageUsed(cloudDriveHandle)
                                        + details->getVersionStorageUsed(vaultHandle)
                                        + details->getVersionStorageUsed(rubbishHandle));
}

void AccountDetailsManager::processInShares(const std::shared_ptr<mega::MegaAccountDetails>& details,
                                            const std::shared_ptr<mega::MegaNodeList>& inShares)
{
    long long inShareSize = 0, inShareFiles = 0, inShareFolders = 0;
    for (int i = 0; i < inShares->size(); i++)
    {
        mega::MegaNode *node = inShares->get(i);
        if (node)
        {
            mega::MegaHandle handle = node->getHandle();
            inShareSize += details->getStorageUsed(handle);
            inShareFiles += details->getNumFiles(handle);
            inShareFolders += details->getNumFolders(handle);
        }
    }
    mPreferences->setInShareStorage(inShareSize);
    mPreferences->setInShareFiles(inShareFiles);
    mPreferences->setInShareFolders(inShareFolders);
}

bool AccountDetailsManager::canContinue(mega::MegaError* error) const
{
    // We need to be both logged AND have fetched the nodes to continue
    // Do not continue if there was an error
    // TODO: investigate getRootNode, getVaultNode and getRubbishNode conditions.
    //       Is this case possible and what should we do? Restart the app?
    if (mPreferences->accountStateInGeneral() != Preferences::STATE_FETCHNODES_OK
        || !mPreferences->logged()
        || error->getErrorCode() != mega::MegaError::API_OK
        || !MegaSyncApp->getRootNode()
        || !MegaSyncApp->getVaultNode()
        || !MegaSyncApp->getRubbishNode())
    {
        return false;
    }

    return true;
}
