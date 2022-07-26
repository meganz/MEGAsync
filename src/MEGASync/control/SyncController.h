#pragma once

#include "model/SyncSettings.h"
#include "model/SyncModel.h"
#include "QTMegaRequestListener.h"

#include "megaapi.h"

#include <QString>
#include <QDir>

/**
 * @brief Sync Controller class
 *
 * Interface object used to control Syncs and report back on results using Qt Signals.
 * Uses SyncModel.h class as the data model.
 *
 */
class SyncController: public QObject, public mega::MegaRequestListener
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
    ~SyncController();

    void addBackup(const QString& localFolder, const QString& syncName = QString());
    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle,
                 const QString& syncName = QString(), mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void removeSync(std::shared_ptr<SyncSetting> syncSetting, const mega::MegaHandle& remoteHandle = mega::INVALID_HANDLE);
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);

    void setMyBackupsDirName();
    void getMyBackupsHandle();
    QString getMyBackupsLocalizedPath();

    void setDeviceName(const QString& name);
    void getDeviceName();

    QString getIsLocalFolderAlreadySyncedMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    Syncability isLocalFolderAlreadySynced(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    static QString getIsLocalFolderAllowedForSyncMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    static Syncability isLocalFolderAllowedForSync(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    static QString getAreLocalFolderAccessRightsOkMsg(const QString& path, const mega::MegaSync::SyncType& syncType);
    static Syncability areLocalFolderAccessRightsOk(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);
    Syncability isLocalFolderSyncable(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message);

    static QString getSyncNameFromPath(const QString& path);

    static const char* DEFAULT_BACKUPS_ROOT_DIRNAME;

signals:
    void syncAddStatus(int errorCode, QString errorMsg, QString name);
    void syncRemoveError(std::shared_ptr<SyncSetting> sync);
    void syncEnableError(std::shared_ptr<SyncSetting> sync);
    void syncDisableError(std::shared_ptr<SyncSetting> sync);

    void setMyBackupsStatus(int errorCode, QString errorMsg);
    void myBackupsHandle(mega::MegaHandle handle);

    void setDeviceDirStatus(int errorCode, QString errorMsg);
    void deviceName(QString deviceName);

protected:
    // override from MegaRequestListener
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* req, mega::MegaError* e) override;

private:
    void ensureDeviceNameIsSetOnRemote();
    void setMyBackupsHandle(mega::MegaHandle handle);
    QString getSyncAPIErrorMsg(int megaError);

    mega::MegaApi* mApi;
    mega::QTMegaRequestListener* mDelegateListener;
    SyncModel* mSyncModel;
    mega::MegaHandle mMyBackupsHandle;
    bool mIsDeviceNameSetOnRemote;
    bool mForceSetDeviceName;
};
