#include "QmlSyncData.h"

QmlSyncData::QmlSyncData(mega::MegaSync* sync):
    handle(sync->getBackupId()),
    type(convertSyncType(sync)),
    name(QString::fromUtf8(sync->getName())),
    status(convertStatus(sync))
{}

QmlSyncData::QmlSyncData(mega::MegaSyncStats* syncStats):
    handle(syncStats->getBackupId()),
    status((syncStats->isScanning()) ? SyncStatus::UPDATING : SyncStatus::UP_TO_DATE)
{}

QmlSyncData::QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api):
    handle(backupInfo->id()),
    type(convertSyncType(backupInfo)),
    name(QString::fromUtf8(backupInfo->name())),
    size(api->getSize(api->getNodeByHandle(backupInfo->root()))),
    dateModified(QDateTime::fromSecsSinceEpoch(static_cast<qint64>(backupInfo->ts()))),
    dateAdded(dateModified),
    status(convertStatus(backupInfo))
{}

QmlSyncData::QmlSyncData(mega::MegaRequest* request, mega::MegaApi* api)
{
    handle = request->getParentHandle();
    type = (request->getParamType() == mega::MegaSync::TYPE_TWOWAY) ? QmlSyncType::SYNC :
                                                                      QmlSyncType::BACKUP;
    name = QString::fromUtf8(request->getName());
    size = api->getSize(api->getNodeByHandle(request->getNodeHandle()));
}

void QmlSyncData::updateFields(const QmlSyncData& other)
{
    if (type == QmlSyncType::UNDEFINED)
    {
        type = other.type;
    }
    if (name.isEmpty() && !other.name.isEmpty())
    {
        name = other.name;
    }
    if (other.size != -1)
    {
        size = other.size;
    }
    if (!other.dateAdded.isNull())
    {
        dateAdded = other.dateAdded;
    }
    if (!other.dateModified.isNull())
    {
        dateModified = other.dateModified;
    }
    status = other.status;
}

QString QmlSyncData::toString() const
{
    return QString::fromUtf8("SyncData \"%1\" - status : %2").arg(name).arg(status);
}

QmlSyncType::Type QmlSyncData::convertSyncType(const mega::MegaSync* sync)
{
    return (sync->getType() == mega::MegaSync::SyncType::TYPE_TWOWAY) ? QmlSyncType::SYNC :
                                                                        QmlSyncType::BACKUP;
}

QmlSyncType::Type QmlSyncData::convertSyncType(const mega::MegaBackupInfo* backupInfo)
{
    return (backupInfo->type() == mega::MegaApi::BACKUP_TYPE_TWO_WAY_SYNC) ? QmlSyncType::SYNC :
                                                                             QmlSyncType::BACKUP;
}

SyncStatus::Value QmlSyncData::convertStatus(const mega::MegaSync* sync)
{
    if (sync->getRunState() == mega::MegaSync::RUNSTATE_PENDING ||
        sync->getRunState() == mega::MegaSync::RUNSTATE_LOADING)
    {
        return SyncStatus::UPDATING;
    }
    else if (sync->getRunState() == mega::MegaSync::RUNSTATE_SUSPENDED)
    {
        return SyncStatus::PAUSED;
    }
    else if (sync->getRunState() == mega::MegaSync::RUNSTATE_DISABLED ||
             sync->getError() != mega::MegaSync::NO_SYNC_ERROR)
    {
        return SyncStatus::STOPPED;
    }
    return SyncStatus::UP_TO_DATE;
}

SyncStatus::Value QmlSyncData::convertStatus(const mega::MegaBackupInfo* backupInfo)
{
    const int apiStatus = backupInfo->status();
    const int syncState = backupInfo->state();

    if (syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_UP ||
        syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_DOWN ||
        syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_FULL ||
        syncState == mega::MegaBackupInfo::BACKUP_STATE_TEMPORARY_DISABLED)
    {
        return SyncStatus::PAUSED;
    }
    else if (syncState == mega::MegaBackupInfo::BACKUP_STATE_FAILED ||
             syncState == mega::MegaBackupInfo::BACKUP_STATE_DELETED)
    {
        return SyncStatus::STOPPED;
    }

    if (apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_INACTIVE)
    {
        return SyncStatus::PAUSED;
    }
    else if (apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_UPTODATE)
    {
        return SyncStatus::UP_TO_DATE;
    }
    else if (apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_PENDING)
    {
        return SyncStatus::UPDATING;
    }
    else if (apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_SYNCING)
    {
        return (syncState == mega::MegaBackupInfo::BACKUP_STATE_TEMPORARY_DISABLED) ?
                   SyncStatus::PAUSED :
                   SyncStatus::UPDATING;
    }
    else
    {
        return SyncStatus::STOPPED;
    }
}
