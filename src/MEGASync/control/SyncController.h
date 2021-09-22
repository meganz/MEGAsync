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
    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle, QString syncName = QString());
    void removeSync(std::shared_ptr<SyncSetting> syncSetting);
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);

    static SyncController& instance();
    void setApi(mega::MegaApi *api);
    void setModel(SyncModel *model);

signals:
    void syncAddError(const QString message);
    void syncRemoveError(std::shared_ptr<SyncSetting> sync);
    void syncEnableError(std::shared_ptr<SyncSetting> sync);
    void syncDisableError(std::shared_ptr<SyncSetting> sync);

protected:
    // override from MegaRequestListener
    virtual void onRequestFinish(mega::MegaApi* mApi, mega::MegaRequest* req, mega::MegaError* e) override;

private:
    SyncController(QObject *parent = nullptr); // singleton class
    static SyncController *mInstance;
    mega::MegaApi *mApi;
    mega::QTMegaRequestListener* mDelegateListener;
    SyncModel* mSyncModel;
};
