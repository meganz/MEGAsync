#include "QmlSyncData.h"

QmlSyncData::QmlSyncData(mega::MegaSync* sync)
{
    handle = sync->getBackupId();
    type = (sync->getType() == mega::MegaSync::SyncType::TYPE_TWOWAY) ? QmlSyncType::SYNC :
                                                                        QmlSyncType::BACKUP;
    name = QString::fromUtf8(sync->getName());

    status = SyncStatus::UP_TO_DATE;
    if (sync->getRunState() == mega::MegaSync::RUNSTATE_PENDING ||
        sync->getRunState() == mega::MegaSync::RUNSTATE_LOADING)
    {
        status = SyncStatus::UPDATING;
    }
    else if (sync->getRunState() == mega::MegaSync::RUNSTATE_SUSPENDED)
    {
        status = SyncStatus::PAUSED;
    }
    else if (sync->getRunState() == mega::MegaSync::RUNSTATE_DISABLED ||
             sync->getError() != mega::MegaSync::NO_SYNC_ERROR)
    {
        status = SyncStatus::STOPPED;
    }
}

QmlSyncData::QmlSyncData(mega::MegaSyncStats* syncStats):
    handle(syncStats->getBackupId()),
    status((syncStats->isScanning()) ? SyncStatus::UPDATING : SyncStatus::UP_TO_DATE)
{}

QmlSyncData::QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api)
{
    handle = backupInfo->id();
    type = (backupInfo->type() == mega::MegaApi::BACKUP_TYPE_TWO_WAY_SYNC) ? QmlSyncType::SYNC :
                                                                             QmlSyncType::BACKUP;
    name = QString::fromUtf8(backupInfo->name());
    size = api->getSize(api->getNodeByHandle(backupInfo->root()));
    dateModified = QDateTime::fromTime_t(backupInfo->ts());
    dateAdded = dateModified;
    status = convertStatus(backupInfo);
}

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

SyncStatus::Value QmlSyncData::convertStatus(const mega::MegaBackupInfo* backupInfo)
{
    const int apiStatus = backupInfo->status();
    const int syncState = backupInfo->state();
    if (syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_UP ||
        syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_DOWN ||
        syncState == mega::MegaBackupInfo::BACKUP_STATE_PAUSE_FULL)
    {
        return SyncStatus::PAUSED;
    }
    else if (syncState == mega::MegaBackupInfo::BACKUP_STATE_FAILED ||
             syncState == mega::MegaBackupInfo::BACKUP_STATE_DELETED)
    {
        return SyncStatus::STOPPED;
    }
    else if (syncState == mega::MegaBackupInfo::BACKUP_STATE_TEMPORARY_DISABLED ||
             apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_INACTIVE)
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
        if (syncState == mega::MegaBackupInfo::BACKUP_STATE_TEMPORARY_DISABLED)
        {
            return SyncStatus::PAUSED;
        }
        else
        {
            return SyncStatus::UPDATING;
        }
    }
    else
    {
        return SyncStatus::STOPPED;
    }
}
