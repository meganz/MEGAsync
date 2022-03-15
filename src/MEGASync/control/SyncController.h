#pragma once

#include <QString>

#include "model/SyncSettings.h"
#include "model/SyncModel.h"
#include "QTMegaRequestListener.h"

#include "megaapi.h"

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
    SyncController(QObject* parent = nullptr);
    ~SyncController();

    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle,
                 const QString& syncName = QString(),
                 mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void removeSync(std::shared_ptr<SyncSetting> syncSetting, bool removeRemote = false);
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);

    void createMyBackupsDir(const QString& name);
    void setMyBackupsDir(mega::MegaHandle handle);

    void setDeviceName(const QString& name);
    void getDeviceName();

    void setBackupsRootDirHandle(mega::MegaHandle handle);
    void getBackupsRootDirHandle();

signals:
    void syncAddStatus(int errorCode, QString errorMsg, QString name);
    void syncRemoveError(std::shared_ptr<SyncSetting> sync);
    void syncEnableError(std::shared_ptr<SyncSetting> sync);
    void syncDisableError(std::shared_ptr<SyncSetting> sync);

    void setMyBackupsDirStatus(int errorCode, QString errorMsg);
    void backupsRootDirHandle(mega::MegaHandle handle);

    void setDeviceDirStatus(int errorCode, QString errorMsg);
    void deviceName(QString deviceName);

protected:
    // override from MegaRequestListener
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* req, mega::MegaError* e) override;

private:
    void ensureDeviceNameIsSetOnRemote();
    QString getSyncAPIErrorMsg(int megaError);

    mega::MegaApi* mApi;
    mega::QTMegaRequestListener* mDelegateListener;
    SyncModel* mSyncModel;
    bool mIsDeviceNameSetOnRemote;
    bool mForceSetDeviceName;
    bool mRemoveRemote;
};
