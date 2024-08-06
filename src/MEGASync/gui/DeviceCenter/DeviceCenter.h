#ifndef DEVICE_CENTER_H
#define DEVICE_CENTER_H

#include "DeviceData.h"
#include "QmlDialogWrapper.h"
#include "QTMegaListener.h"
#include "SyncModel.h"

class DeviceCenter: public QMLComponent, public mega::MegaListener
{
    Q_OBJECT
    Q_PROPERTY(SyncModel* syncModel MEMBER mSyncModel CONSTANT)

public:
    explicit DeviceCenter(QObject* parent = 0);
    virtual ~DeviceCenter();

    void onRequestFinish(mega::MegaApi* api,
                         mega::MegaRequest* request,
                         mega::MegaError* e) override;
    void onSyncStateChanged(mega::MegaApi*, mega::MegaSync* sync) override;
    void onSyncStatsUpdated(mega::MegaApi* api, mega::MegaSyncStats* syncStats) override;

    QUrl getQmlUrl() override;
    QString contextName() override;
    static void registerQmlModules();

    Q_INVOKABLE void openAddBackupDialog();
    Q_INVOKABLE QString getThisDeviceId();
    Q_INVOKABLE void retrieveDeviceData(const QString& deviceId);
    Q_INVOKABLE QString getSizeString(unsigned long long bytes);
    Q_INVOKABLE DeviceOs::Os getCurrentOS();
signals:
    void deviceNameReceived(QString deviceName);
    void deviceDataUpdated(DeviceData data);

private:
    using BackupList = QList<const mega::MegaBackupInfo*>;
    void updateLocalData(mega::MegaBackupInfoList& backupList);
    void updateLocalData(const QmlSyncData& syncObj);
    void updateDeviceData();

    BackupList filterBackupList(const char* deviceId, mega::MegaBackupInfoList& backupList);

    mega::MegaApi* mMegaApi;
    SyncModel* mSyncModel;
    QPointer<mega::QTMegaListener> mDelegateListener;
    QString mDeviceIdFromLastRequest;

    DeviceData mCachedDeviceData;
};

#endif // DEVICE_CENTER_H
