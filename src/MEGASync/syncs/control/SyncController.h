#pragma once

#include "syncs/control/SyncSettings.h"
#include "syncs/control/SyncInfo.h"

#include "megaapi.h"

#include <QString>
#include <QDir>

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

    SyncController(QObject* parent = nullptr);
    ~SyncController(){}

    void addBackup(const QString& localFolder, const QString& syncName = QString());
    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle,
                 const QString& syncName = QString(), mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void removeSync(std::shared_ptr<SyncSettings> syncSetting, const mega::MegaHandle& remoteHandle = mega::INVALID_HANDLE);
    void enableSync(std::shared_ptr<SyncSettings> syncSetting);
    void disableSync(std::shared_ptr<SyncSettings> syncSetting);

    // Local folder checks
    static QString getIsLocalFolderAlreadySyncedMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    static Syncability isLocalFolderAlreadySynced(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    static QString getIsLocalFolderAllowedForSyncMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    static Syncability isLocalFolderAllowedForSync(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    static QString getAreLocalFolderAccessRightsOkMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    static Syncability areLocalFolderAccessRightsOk(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    static Syncability isLocalFolderSyncable(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);

    // Remote folder check
    static Syncability isRemoteFolderSyncable(std::shared_ptr<mega::MegaNode> node, QString& message);

    static QString getSyncNameFromPath(const QString& path);

    //Error strings
    static QString getErrStrCurrentBackupOverExistingBackup();
    static QString getErrStrCurrentBackupInsideExistingBackup();

signals:
    void syncAddStatus(int errorCode, int syncErrorCode, QString errorMsg, QString name);
    void syncRemoveError(std::shared_ptr<mega::MegaError> err);
    void syncEnableError(std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode);
    void syncDisableError(std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode);
    void backupMoveOrRemoveRemoteFolderError(std::shared_ptr<mega::MegaError> err);

private:
    void createPendingBackups();
    static QString getSyncAPIErrorMsg(int megaError);
    static QString getSyncTypeString(const mega::MegaSync::SyncType& syncType);
    QMap<QString, QString> mPendingBackups;

    mega::MegaApi* mApi;

    //Only use const methods
    const SyncInfo* mSyncInfo;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaError>)
Q_DECLARE_METATYPE(mega::MegaSync::Error)
