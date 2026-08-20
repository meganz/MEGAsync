#ifndef SYNC_CONTROLLER_H
#define SYNC_CONTROLLER_H

#include "AppStatsEvents.h"
#include "megaapi.h"
#include "SyncInfo.h"
#include "SyncSettings.h"

#include <QDir>
#include <QSet>
#include <QString>

#include <functional>
#include <optional>

/**
 * @brief Sync Controller class
 *
 * Interface object used to control Syncs and report back on results using Qt Signals.
 * Uses SyncInfo.h class as the data model.
 *
 */

class SyncController: public QObject
{
    Q_OBJECT

public:

    enum Syncability
    {
       CAN_SYNC = 0,
       WARN_SYNC,
       CANT_SYNC,
    };

    struct SyncConfig
    {
        QString localFolder;
        QString remoteFolder;
        mega::MegaHandle remoteHandle = mega::INVALID_HANDLE;
        QString syncName;
        mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY;
    };

    static SyncController& instance()
    {
        static SyncController instance;
        return instance;
    }

    SyncController(const SyncController&) = delete;
    SyncController& operator=(const SyncController&) = delete;
    ~SyncController(){}

    void addBackup(const QString& localFolder, const QString& syncName);
    void addSync(SyncConfig& sync);
    void removeSync(std::shared_ptr<SyncSettings> syncSetting, const mega::MegaHandle& remoteHandle = mega::INVALID_HANDLE);
    void moveOrDeleteRemovedBackupData(std::shared_ptr<SyncSettings> syncSetting,
                                       const mega::MegaHandle& remoteHandle = mega::INVALID_HANDLE);
    void prevalidateSync(SyncConfig& sync);

    void setSyncToRun(std::shared_ptr<SyncSettings> syncSetting);
    void setSyncToPause(std::shared_ptr<SyncSettings> syncSetting);
    void setSyncToSuspend(std::shared_ptr<SyncSettings> syncSetting);
    void setSyncToDisabled(std::shared_ptr<SyncSettings> syncSetting);
    // Change state and then set to running
    void resetSync(std::shared_ptr<SyncSettings> syncSetting,
                   mega::MegaSync::SyncRunningState initialState);

    // Suspends the sync, runs duringPause once it has actually stopped, then resumes it.
    // Use this instead of resetSync() when the local folder is modified in a way that
    // must not be seen by a still-running sync (e.g. removing .megaignore).
    void pauseRunAndResume(std::shared_ptr<SyncSettings> syncSetting,
                           std::function<void()> duringPause);

    // Local folder checks
    QString getIsLocalFolderAlreadySyncedMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    Syncability isLocalFolderAlreadySynced(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    QString getIsLocalFolderAllowedForSyncMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    Syncability isLocalFolderAllowedForSync(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    Syncability isLocalFolderSyncable(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    Syncability isLocalFolderSyncable(const QString& path,
                                      const mega::MegaSync::SyncType& syncType);

    // Remote folder check
    Syncability isRemoteFolderSyncable(std::shared_ptr<mega::MegaNode> node, QString& message);

    QString getSyncNameFromPath(const QString& path);

    //Error strings
    QString getErrStrCurrentBackupOverExistingBackup();
    QString getErrStrCurrentBackupInsideExistingBackup();
    virtual QString getErrorString(int errorCode, int syncErrorCode) const;
    QString getRemoteFolderErrorMessage(int errorCode, int syncErrorCode);

    // Check is sync folder is case sensitive
    Qt::CaseSensitivity isSyncCaseSensitive(mega::MegaHandle backupId);

signals:
    void syncAddStatus(int errorCode, int syncErrorCode, QString name);
    void syncPrevalidateStatus(int errorCode, int syncErrorCode);
    void syncRemoveBegins(mega::MegaHandle);
    void syncRemoveEnds(mega::MegaHandle);
    void syncRemoveStatus(int errorCode);
    void syncRemoveError(std::shared_ptr<mega::MegaError> err);
    void signalSyncOperationBegins();
    void signalSyncOperationEnds();
    void signalSyncOperationError(std::shared_ptr<SyncSettings> sync);
    void backupMoveOrRemoveRemoteFolderError(std::shared_ptr<mega::MegaError> err);

protected:
    SyncController(QObject* parent = nullptr);
    virtual QString getSyncAPIErrorMsg(int megaError) const;

private:
    void updateSyncSettings(const mega::MegaError& e, std::shared_ptr<SyncSettings> syncSetting);
    void createPendingBackups();

    static QString getSyncTypeString(const mega::MegaSync::SyncType& syncType);

    static QString getDescription(SyncInfo::SyncOrigin origin);

    QMap<QString, QString> mPendingBackups;

    //Sync/Backup operation signals
    void syncOperationBegins();
    void syncOperationEnds();
    uint mActiveOperations;

    // backupIds currently inside a pauseRunAndResume() call, so a reentrant call for the
    // same sync (e.g. a second click processed while the first is still blocked waiting
    // on the SDK) can be rejected instead of racing it.
    QSet<mega::MegaHandle> mPauseRunAndResumeInProgress;

    mega::MegaApi* mApi;

    SyncInfo* mSyncInfo;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaError>)
Q_DECLARE_METATYPE(mega::MegaSync::Error)
#endif
