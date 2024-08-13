#ifndef QMLSYNCDATA_H
#define QMLSYNCDATA_H

#include "SyncStatus.h"

#include <megaapi.h>
#include <QDateTime>

struct QmlSyncData
{
    QmlSyncData();
    QmlSyncData(mega::MegaSync* sync);
    QmlSyncData(mega::MegaSyncStats* syncStats);
    QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api);
    QmlSyncData(mega::MegaRequest* request, mega::MegaApi* api);

    void updateFields(const QmlSyncData& other);
    QString toString() const;

    mega::MegaHandle handle = mega::INVALID_HANDLE;
    QmlSyncType::Type type = QmlSyncType::UNDEFINED;
    QString name = QString::fromLatin1("");
    qint64 size = -1;
    QDateTime dateAdded;
    QDateTime dateModified;
    SyncStatus::Value status = SyncStatus::UP_TO_DATE;

private:
    static SyncStatus::Value convertStatus(const mega::MegaBackupInfo* backupInfo);
};

#endif // QMLSYNCDATA_H
