#ifndef DEVICE_CENTER_H
#define DEVICE_CENTER_H

#include "QmlDialogWrapper.h"
#include "SyncModel.h"

class DeviceCenter: public QMLComponent
{
    Q_OBJECT
    Q_PROPERTY(SyncModel* syncModel MEMBER mSyncModel CONSTANT)

public:
    explicit DeviceCenter(QObject* parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;
    static void registerQmlModules();
    Q_INVOKABLE void openAddBackupDialog();

private:
    SyncModel* mSyncModel;
};

#endif // DEVICE_CENTER_H
