#include "AccountDetailsManager.h"

#include "Utilities.h"
#include "StatsEventHandler.h"
#include "DialogOpener.h"
#include "SettingsDialog.h"

AccountDetailsManager::AccountDetailsManager(mega::MegaApi* megaApi, QObject* parent)
    : QObject(parent)
    , mMegaApi(megaApi)
    , mPreferences(Preferences::instance())
    , mFlags(std::make_unique<UserStatsFlags>())
    , mInflightUserStats(UserStats<bool>(mFlags))
    , mQueuedUserStats(UserStats<bool>(mFlags))
    , mLastRequestUserStats(UserStats<long long>(mFlags))
{
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

void AccountDetailsManager::handleAccountDetailsReply(mega::MegaRequest* request,
                                                      mega::MegaError* error)
{
    if(!canContinue(error))
    {
        return;
    }

    mFlags.parse(request->getNumDetails());
    mInflightUserStats.updateWithValue(false);

    //Account details retrieved, update the preferences and the information dialog
    shared_ptr<mega::MegaAccountDetails> details(request->getMegaAccountDetails());

    MegaSyncApp->pushToThreadPool([=]()
    {
        shared_ptr<mega::MegaNodeList> inShares(mFlags->storage ? mMegaApi->getInShares() : nullptr);

        Utilities::queueFunctionInAppThread([=]()
        {
            processProFlags(details.get());
            processStorageFlag(details.get(), inShares.get());
            processTransferFlags(details.get());

            if (infoDialog)
            {
                infoDialog->setUsage();
                infoDialog->setAccountType(mPreferences->accountType());
            }
        });
    });
}

void AccountDetailsManager::processProFlag(const std::shared_ptr<mega::MegaAccountDetails>& details)
{
    if (!mFlags->pro)
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
            proExpirityTimer.stop();
            const long long interval = qMax(0LL, details->getProExpiration() * 1000 - QDateTime::currentMSecsSinceEpoch());
            proExpirityTimer.setInterval(static_cast<int>(interval));
            proExpirityTimer.start();
        }
    }
    else
    {
        mPreferences->setProExpirityTime(0);
        proExpirityTimer.stop();
    }

    notifyAccountObservers();
}

void AccountDetailsManager::processStorageFlag(const std::shared_ptr<mega::~MegaAccountDetails>& details,
                                               const std::shared_ptr<mega::MegaNodeList>& inShares)
{
    if (!mFlags->storage)
    {
        return;
    }

    mPreferences->setTotalStorage(details->getStorageMax());
    MegaSyncApp->updateUsedStorage(true);

    // Set in preferences the storage, files and folder for the cloud drive, rubbish and vault.
    processNodes();

    // For versions, match the webclient by only counting the user's own nodes.
    // Versions in inshares are not cleared by 'clear versions'.
    // Also the no-parameter getVersionStorageUsed() double counts the versions in outshares.
    // Inshare storage count should include versions.
    mPreferences->setVersionsStorage(details->getVersionStorageUsed(cloudDriveHandle)
                                        + details->getVersionStorageUsed(vaultHandle)
                                        + details->getVersionStorageUsed(rubbishHandle));

    // Inshares
    processInShares(inShares);


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
    if (!transfer)
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

void AccountDetailsManager::processNode(const mega::MegaNode* node,
                                        const std::shared_ptr<mega::MegaAccountDetails>& details,
                                        std::function<void (mega::MegaHandle)> setStorageUsed,
                                        std::function<void (mega::MegaHandle)> setNumFiles,
                                        std::function<void (mega::MegaHandle)> setNumFolders)
{
    mega::MegaHandle handle = node ? node->getHandle() : mega::INVALID_HANDLE;
    setStorageUsed(details->getStorageUsed(handle));
    setNumFiles(details->getNumFiles(handle));
    setNumFolders(details->getNumFolders(handle));
}

void AccountDetailsManager::processNodes()
{
    // Cloud Drive
    processNode(MegaSyncApp->getRootNode(),
                details,
                std::bind(&Preferences::setCloudDriveStorage, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setCloudDriveFiles, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setCloudDriveFolders, mPreferences, std::placeholders::_1));

    // Vault
    processNode(MegaSyncApp->getVaultNode(),
                details,
                std::bind(&Preferences::setVaultStorage, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setVaultFiles, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setVaultFolders, mPreferences, std::placeholders::_1));

    // Rubbish
    processNode(MegaSyncApp->getRubbishNode(),
                details,
                std::bind(&Preferences::setRubbishStorage, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setRubbishFiles, mPreferences, std::placeholders::_1),
                std::bind(&Preferences::setRubbishFolders, mPreferences, std::placeholders::_1));
}

void AccountDetailsManager::processInShares(const std::shared_ptr<mega::MegaNodeList>& inShares)
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
