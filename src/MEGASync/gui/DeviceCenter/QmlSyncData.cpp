#include "QmlSyncData.h"

QmlSyncData::QmlSyncData():
    handle(mega::INVALID_HANDLE),
    type(QString::fromLatin1("UNDEFINED")),
    name(),
    size(-1),
    dateAdded(),
    dateModified()
{}

QmlSyncData::QmlSyncData(mega::MegaSync* sync)
{
    handle = sync->getBackupId();
    type = (sync->getType() == mega::MegaSync::SyncType::TYPE_TWOWAY) ? QString::fromUtf8("Sync") :
                                                                        QString::fromUtf8("Backup");
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

QmlSyncData::QmlSyncData(mega::MegaSyncStats* syncStats)
{
    handle = syncStats->getBackupId();
    status = SyncStatus::UP_TO_DATE;
    if (syncStats->isSyncing() || syncStats->isScanning())
    {
        status = SyncStatus::UPDATING;
    }
}

QmlSyncData::QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api)
{
    handle = backupInfo->id();
    type = (backupInfo->type() == mega::MegaApi::BACKUP_TYPE_TWO_WAY_SYNC) ?
               QString::fromUtf8("Sync") :
               QString::fromUtf8("Backup");
    name = QString::fromUtf8(backupInfo->name());
    size = api->getSize(api->getNodeByHandle(backupInfo->root()));
    dateModified = QDateTime::fromTime_t(backupInfo->ts());
    dateAdded = dateModified;
    status = convertStatus(backupInfo->status());
}

QmlSyncData::QmlSyncData(mega::MegaRequest* request, mega::MegaApi* api)
{
    handle = request->getParentHandle();
    type = QString::number(request->getParamType());
    name = QString::fromUtf8(request->getName());
    size = api->getSize(api->getNodeByHandle(request->getNodeHandle()));
}

void QmlSyncData::updateFields(const QmlSyncData& other)
{
    if (other.type != QString::fromLatin1("UNDEFINED"))
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

SyncStatus::Value QmlSyncData::convertStatus(const int apiStatus)
{
    if (apiStatus <= mega::MegaBackupInfo::BACKUP_STATUS_UPTODATE)
    {
        return SyncStatus::UP_TO_DATE;
    }
    else if (apiStatus <= mega::MegaBackupInfo::BACKUP_STATUS_PENDING)
    {
        return SyncStatus::UPDATING;
    }
    else if (apiStatus == mega::MegaBackupInfo::BACKUP_STATUS_INACTIVE)
    {
        return SyncStatus::PAUSED;
    }
    else
    {
        return SyncStatus::STOPPED;
    }
}
