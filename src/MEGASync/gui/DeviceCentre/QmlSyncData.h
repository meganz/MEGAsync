#ifndef QMLSYNCDATA_H
#define QMLSYNCDATA_H

#include "megaapi.h"
#include "SyncStatus.h"

#include <QDateTime>

struct QmlSyncData
{
    QmlSyncData() = default;
    QmlSyncData(mega::MegaSync* sync);
    QmlSyncData(mega::MegaSyncStats* syncStats);
    QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api);
    QmlSyncData(mega::MegaRequest* request, mega::MegaApi* api);

    void updateFields(const QmlSyncData& other);
    QString toString() const;

    mega::MegaHandle syncID = mega::INVALID_HANDLE;
    mega::MegaHandle nodeHandle = mega::INVALID_HANDLE;
    QString localFolder;
    QmlSyncType::Type type = QmlSyncType::UNDEFINED;
    QString name;
    qint64 size = -1;
    QDateTime dateModified;
    SyncStatus::Value status = SyncStatus::UP_TO_DATE;

private:
    static QmlSyncType::Type convertSyncType(const mega::MegaSync* sync);
    static QmlSyncType::Type convertSyncType(const mega::MegaBackupInfo* backupInfo);
    static SyncStatus::Value convertStatus(const mega::MegaSync* sync);
    static SyncStatus::Value convertStatus(const mega::MegaBackupInfo* backupInfo);
};

#endif // QMLSYNCDATA_H
