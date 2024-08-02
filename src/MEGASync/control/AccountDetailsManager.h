#ifndef ACCOUNT_DETAILS_MANAGER_H
#define ACCOUNT_DETAILS_MANAGER_H

#include "Utilities.h"

#include "megaapi.h"

#include <QObject>
#include <QTimer>

#include <memory>
#include <functional>

class Preferences;

class AccountDetailsManager
    : public QObject
    , public StorageDetailsObserved
    , public BandwidthDetailsObserved
    , public AccountDetailsObserved
{
    Q_OBJECT

public:
    AccountDetailsManager(mega::MegaApi* megaApi,
                          QObject* parent = nullptr);
    virtual ~AccountDetailsManager() = default;

    void reset();

    void onRequestFinish(mega::MegaRequest* request,
                         mega::MegaError* error);

    void updateUserStats(bool storage, bool transfer, bool pro, bool force, int source);
    void periodicUpdate();

signals:
    void accountDetailsUpdated();

private:
    struct UserStatsFlags
    {
        UserStatsFlags();
        UserStatsFlags(bool storage, bool transfer, bool pro);
        ~UserStatsFlags() = default;

        void parse(int flags);

        bool storage;
        bool transfer;
        bool pro;
    };

    template<typename Type>
    class UserStats
    {

    public:
        UserStats() = default;
        ~UserStats() = default;

        void updateWithValue(const UserStatsFlags& flags, Type value);
        void updateWithValue(Type value);

        Type storageValue() const;
        Type transferValue() const;
        Type proValue() const;

    private:
        Type mStorageValue;
        Type mTransferValue;
        Type mProValue;

    };

    mega::MegaApi* mMegaApi;
    std::shared_ptr<Preferences> mPreferences;
    UserStatsFlags mFlags;
    UserStats<bool> mInflightUserStats;
    UserStats<bool> mQueuedUserStats;
    UserStats<long long> mLastRequestUserStats;
    int mQueuedStorageUserStatsReason;
    QTimer mProExpirityTimer;

    void handleAccountDetailsReply(mega::MegaRequest* request,
                                   mega::MegaError* error);
    bool canContinue(mega::MegaError* error) const;
    void processProFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processStorageFlag(const std::shared_ptr<mega::MegaAccountDetails>& details,
                            const std::shared_ptr<mega::MegaNodeList>& inShares);
    void processTransferFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    mega::MegaHandle processNode(const std::shared_ptr<mega::MegaNode>& node,
                                 const std::shared_ptr<mega::MegaAccountDetails>& details,
                                 std::function<void(mega::MegaHandle)> setStorageUsed,
                                 std::function<void(mega::MegaHandle)> setNumFiles,
                                 std::function<void(mega::MegaHandle)> setNumFolders);
    void processNodesAndVersionsStorage(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processInShares(const std::shared_ptr<mega::MegaAccountDetails>& details,
                         const std::shared_ptr<mega::MegaNodeList>& inShares);

};

#endif // ACCOUNT_DETAILS_MANAGER_H
