#ifndef ACCOUNT_DETAILS_MANAGER_H
#define ACCOUNT_DETAILS_MANAGER_H

#include "Preferences.h"

#include "megaapi.h"

#include <QObject>

#include <memory>
#include <functional>

struct UserStatsFlags
{
    void parse(int flags)
    {
        storage = flags & 0x01;
        transfer = flags & 0x02;
        pro = flags & 0x04;
    }

    bool storage;
    bool transfer;
    bool pro;
};

template<typedef Type>
class UserStats
{

public:
    UserStats(UserStatsFlags* flags)
        : mFlags(flags)
        , mStorageFlag(false)
        , mTransferFlag(false)
        , mProFlag(false)
    {
    }

    void updateWithValue(Type value)
    {
        if (mFlags->storage)
        {
            mStorageValue = value;
        }
        if (mFlags->transfer)
        {
            mTransferValue = value;
        }
        if (mFlags->pro)
        {
            mProValue = value;
        }
    }

private:
    UserStatsFlags* mFlags;
    Type mStorageValue;
    Type mTransferValue;
    Type mProValue;

};

class AccountDetailsManager : public QObject
{
    Q_OBJECT

public:
    AccountDetailsManager(mega::MegaApi* megaApi, QObject* parent = nullptr);
    virtual ~AccountDetailsManager();

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<Preferences> mPreferences;
    std::unique_ptr<UserStatsFlags> mFlags;
    UserStats<bool> mInflightUserStats;
    UserStats<bool> mQueuedUserStats;
    UserStats<long long> mLastRequestUserStats;

    void handleAccountDetailsReply(mega::MegaRequest* request,
                                   mega::MegaError* error);
    bool canContinue(mega::MegaError* error) const;
    void processProFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processStorageFlag(const std::shared_ptr<mega::MegaAccountDetails>& details,
                            const std::shared_ptr<mega::MegaNodeList>& inShares);
    void processTransferFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processNode(const mega::MegaNode* node,
                     const std::shared_ptr<mega::MegaAccountDetails>& details,
                     std::function<void(mega::MegaHandle)> setStorageUsed,
                     std::function<void(mega::MegaHandle)> setNumFiles,
                     std::function<void(mega::MegaHandle)> setNumFolders);
    void processNodes();
    void processInShares(const std::shared_ptr<mega::MegaNodeList>& inShares);

};

#endif // ACCOUNT_DETAILS_MANAGER_H
