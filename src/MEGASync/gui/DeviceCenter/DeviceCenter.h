#ifndef DEVICE_CENTER_H
#define DEVICE_CENTER_H

#include "DeviceData.h"
#include "DeviceModel.h"
#include "QmlDialogWrapper.h"
#include "QTMegaListener.h"
#include "SyncModel.h"

#include <QTimer>

class DeviceCenter: public QMLComponent, public mega::MegaListener
{
    Q_OBJECT
    Q_PROPERTY(DeviceData deviceData MEMBER mCachedDeviceData CONSTANT)

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

    Q_INVOKABLE QString getCurrentDeviceId();
    Q_INVOKABLE void retrieveDeviceData(const QString& deviceId);
    Q_INVOKABLE QString getSizeString(long long bytes) const;
    Q_INVOKABLE DeviceOs::Os getCurrentOS();
    Q_INVOKABLE void openAddBackupDialog();
    Q_INVOKABLE void openAddSyncDialog();
    Q_INVOKABLE DeviceModel* getDeviceModel() const;
    Q_INVOKABLE SyncModel* getSyncModel() const;

signals:
    void deviceNameReceived(QString deviceName);
    void deviceDataUpdated();

private:
    using BackupList = QList<const mega::MegaBackupInfo*>;
    void updateLocalData(const mega::MegaBackupInfoList& backupList);
    void updateLocalData(const QmlSyncData& syncObj);
    void updateDeviceData();

    BackupList filterBackupList(const char* deviceId, const mega::MegaBackupInfoList& backupList);

    mega::MegaApi* mMegaApi;
    SyncModel* mSyncModel;
    QPointer<mega::QTMegaListener> mDelegateListener;
    QString mDeviceIdFromLastRequest;
    QTimer mSizeInfoTimer;

    DeviceData mCachedDeviceData;
    DeviceModel* mDeviceModel;
};

#endif // DEVICE_CENTER_H
