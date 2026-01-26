#ifndef DEVICE_NAME_CHECKER_H
#define DEVICE_NAME_CHECKER_H

#include "DeviceNames.h"
#include "megaapi.h"
#include "MyBackupsHandle.h"

#include <QSet>
#include <QThread>

class DeviceNameChecker: public QThread
{
    Q_OBJECT

public:
    explicit DeviceNameChecker(QObject* parent, QString deviceNameCandidate);
    void run() override;

signals:
    void deviceNameCheck(bool isValid);

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandle;
    std::shared_ptr<UserAttributes::DeviceNames> mDeviceName;
    QSet<QString> mBackupDeviceNames;
    int mPendingRequests = 0;
    QString mDeviceNameCandidate;

    void fetchBackupDeviceNames();
    void updateReadyCondition();
    bool checkDeviceName(QString deviceName);
};

#endif
