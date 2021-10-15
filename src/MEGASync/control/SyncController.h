#pragma once

#include <QString>

#include "SyncSettings.h"
#include "SyncModel.h"
#include "megaapi.h"
#include "QTMegaRequestListener.h"


/**
 * @brief Sync Controller class
 *
 * Class used to control Syncs and report back on errors using Qt Signals. Uses SyncModel.h class as
 * the data model.
 *
 */
class SyncController: public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle,
                 const QString& syncName = QString(),
                 mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void removeSync(std::shared_ptr<SyncSetting> syncSetting);
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);

    void createMyBackupsDir(const QString& name);
    void setMyBackupsDir(mega::MegaHandle handle);
    void setDeviceName(const QString& name);
    void getDeviceName();
    void ensureDeviceNameIsSetOnRemote();

    void setBackupsRootDirHandle(mega::MegaHandle handle);
    void getBackupsRootDirHandle();

    static SyncController& instance();
    void setApi(mega::MegaApi* api);
    void setModel(SyncModel* model);

signals:
    void syncRemoveError(std::shared_ptr<SyncSetting> sync);
    void syncEnableError(std::shared_ptr<SyncSetting> sync);
    void syncDisableError(std::shared_ptr<SyncSetting> sync);
    void syncAddStatus(int errorCode, QString errorMsg);

    void setMyBackupsDirStatus(int errorCode, QString errorMsg);
    void setDeviceDirStatus(int errorCode, QString errorMsg);
    void deviceName(QString deviceName);
    void backupsRootDirHandle(mega::MegaHandle handle);


protected:
    // override from MegaRequestListener
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* req, mega::MegaError* e) override;

private:
    SyncController(QObject* parent = nullptr); // singleton class
    static SyncController* mInstance;
    mega::MegaApi* mApi;
    mega::QTMegaRequestListener* mDelegateListener;
    SyncModel* mSyncModel;
    bool mIsDeviceNameSetOnRemote;
    bool mForceSetDeviceName;
};
