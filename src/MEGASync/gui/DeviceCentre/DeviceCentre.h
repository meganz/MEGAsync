#ifndef DEVICE_CENTER_H
#define DEVICE_CENTER_H

#include "DeviceData.h"
#include "DeviceModel.h"
#include "QmlDialogWrapper.h"
#include "QTMegaListener.h"
#include "SyncModel.h"

#include <QTimer>

class DeviceCentre: public QMLComponent, public mega::MegaListener
{
    Q_OBJECT
    Q_PROPERTY(DeviceData deviceData MEMBER mCachedDeviceData CONSTANT)
    Q_PROPERTY(int rowCount READ getRowCount NOTIFY rowCountChanged)

public:
    explicit DeviceCentre(QObject* parent = 0);
    virtual ~DeviceCentre();

    void onRequestFinish(mega::MegaApi* api,
                         mega::MegaRequest* request,
                         mega::MegaError* e) override;
    void onSyncStateChanged(mega::MegaApi*, mega::MegaSync* sync) override;
    void onSyncStatsUpdated(mega::MegaApi* api, mega::MegaSyncStats* syncStats) override;
    void onSyncDeleted(mega::MegaApi* api, mega::MegaSync* sync) override;

    QUrl getQmlUrl() override;
    QString contextName() override;
    static void registerQmlModules();

    Q_INVOKABLE void retrieveDeviceData(const QString& deviceId);
    Q_INVOKABLE QString getSizeString(long long bytes) const;
    Q_INVOKABLE DeviceOs::Os getCurrentOS();
    Q_INVOKABLE void openAddBackupDialog();
    Q_INVOKABLE void openAddSyncDialog();
    Q_INVOKABLE DeviceModel* getDeviceModel() const;
    Q_INVOKABLE SyncModel* getSyncModel() const;
    Q_INVOKABLE void renameCurrentDevice(const QString& newName);
    Q_INVOKABLE void openInMega(int row) const;
    Q_INVOKABLE void showInFolder(int row) const;
    Q_INVOKABLE void pauseSync(int row) const;
    Q_INVOKABLE void resumeSync(int row) const;
    Q_INVOKABLE void rescanSync(int row, bool deepRescan = true) const;
    Q_INVOKABLE void manageExclusions(int row) const;
    Q_INVOKABLE void stopSync(int row) const;
    Q_INVOKABLE void rebootSync(int row) const;
    Q_INVOKABLE bool deviceNameAlreadyExists(const QString& name) const;
    Q_INVOKABLE int getRowCount() const;

    Q_INVOKABLE void learnMore() const;
    Q_INVOKABLE void applyPreviousExclusionRules() const;
    Q_INVOKABLE void onSmartModeSelected() const;
    Q_INVOKABLE void onAdvancedModeSelected() const;
    Q_INVOKABLE bool isSmartModeSelected() const;

signals:
    void deviceNameReceived(QString deviceName);
    void deviceDataUpdated();
    void rowCountChanged();

private:
    using BackupList = QList<const mega::MegaBackupInfo*>;
    void updateLocalData(const mega::MegaBackupInfoList& backupList);
    void updateLocalData(const QmlSyncData& syncObj);
    void updateDeviceData();
    void requestDeviceNames(const mega::MegaBackupInfoList& backupList) const;

    void changeSyncStatus(int row, std::function<void(std::shared_ptr<SyncSettings>)> action) const;

    BackupList filterBackupList(const char* deviceId, const mega::MegaBackupInfoList& backupList);

    mega::MegaApi* mMegaApi;
    SyncModel* mSyncModel;
    std::unique_ptr<mega::QTMegaListener> mDelegateListener;
    QString mDeviceIdFromLastRequest;
    QTimer mSizeInfoTimer;

    DeviceData mCachedDeviceData;
    DeviceModel* mDeviceModel;
};

#endif // DEVICE_CENTER_H
