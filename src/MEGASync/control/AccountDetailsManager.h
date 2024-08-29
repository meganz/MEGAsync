#ifndef ACCOUNT_DETAILS_MANAGER_H
#define ACCOUNT_DETAILS_MANAGER_H

#include "Utilities.h"

#include "megaapi.h"

#include <QObject>
#include <QTimer>

#include <memory>
#include <functional>

class Preferences;

namespace mega
{
class QTMegaRequestListener;
}

class AccountDetailsManager
    : public QObject
    , public StorageDetailsObserved
    , public BandwidthDetailsObserved
    , public AccountDetailsObserved
{
    Q_OBJECT

public:
    enum class Flag
    {
        NONE = 0x0,
        STORAGE = 0x01,
        TRANSFER = 0x02,
        PRO = 0x04,
        STORAGE_PRO = STORAGE | PRO, // 0x05
        TRANSFER_PRO = TRANSFER | PRO, // 0x06
        ALL = STORAGE | TRANSFER | PRO, // 0x07
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    static AccountDetailsManager* instance();

    AccountDetailsManager(const AccountDetailsManager&) = delete;
    AccountDetailsManager& operator=(const AccountDetailsManager&) = delete;

    void init(mega::MegaApi* megaApi);
    void reset();

    void onRequestFinish(mega::MegaRequest* request,
                         mega::MegaError* error);

    void updateUserStats(const Flags& flags, bool force, int source);
    void periodicUpdate();

signals:
    void accountDetailsUpdated();

private:
    template<typename Type>
    class UserStats
    {

    public:
        UserStats() = default;
        ~UserStats() = default;

        void updateWithValue(const Flags& flags, Type value);
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
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<Preferences> mPreferences;
    Flags mFlags;
    UserStats<bool> mInflightUserStats;
    UserStats<bool> mQueuedUserStats;
    UserStats<long long> mLastRequestUserStats;
    int mQueuedStorageUserStatsReason;
    QTimer mProExpirityTimer;

    AccountDetailsManager(QObject* parent = nullptr);

    void handleAccountDetailsReply(mega::MegaRequest* request,
                                   mega::MegaError* error);
    bool canContinue(mega::MegaError* error) const;
    void processProFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processStorageFlag(const std::shared_ptr<mega::MegaAccountDetails>& details,
                            const std::shared_ptr<mega::MegaNodeList>& inShares);
    void processTransferFlag(const std::shared_ptr<mega::MegaAccountDetails>& details);
    mega::MegaHandle processNode(const std::shared_ptr<mega::MegaNode>& node,
                                 const std::shared_ptr<mega::MegaAccountDetails>& details,
                                 std::function<void (long long)> setStorageUsed,
                                 std::function<void (long long)> setNumFiles,
                                 std::function<void (long long)> setNumFolders);
    void processNodesAndVersionsStorage(const std::shared_ptr<mega::MegaAccountDetails>& details);
    void processInShares(const std::shared_ptr<mega::MegaAccountDetails>& details,
                         const std::shared_ptr<mega::MegaNodeList>& inShares);
    void checkInflightUserStats(Flags& flags);

    static long long getLastRequest(const Flags& flags,
                                    const UserStats<long long>& lastRequestUserStats);
    static AccountDetailsManager* mInstance;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AccountDetailsManager::Flags)

#endif // ACCOUNT_DETAILS_MANAGER_H
