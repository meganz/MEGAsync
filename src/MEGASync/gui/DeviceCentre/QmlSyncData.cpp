#include "QmlSyncData.h"

#include "SyncInfo.h"

QmlSyncData::QmlSyncData(mega::MegaSync* sync):
    syncID(sync->getBackupId()),
    nodeHandle(sync->getMegaHandle()),
    localFolder(QString::fromUtf8(sync->getLocalFolder())),
    type(convertSyncType(sync)),
    name(QString::fromUtf8(sync->getName())),
    status(convertStatus(sync))
{}

QmlSyncData::QmlSyncData(mega::MegaSyncStats* syncStats):
    syncID(syncStats->getBackupId()),
    status((syncStats->isScanning()) ? SyncStatus::UPDATING : SyncStatus::UP_TO_DATE)
{}

QmlSyncData::QmlSyncData(const mega::MegaBackupInfo* backupInfo, mega::MegaApi* api):
    syncID(backupInfo->id()),
    nodeHandle(backupInfo->root()),
    localFolder(QString::fromUtf8(backupInfo->localFolder())),
    type(convertSyncType(backupInfo)),
    name(QString::fromUtf8(backupInfo->name())),
    size(api->getSize(api->getNodeByHandle(backupInfo->root()))),
    dateModified(QDateTime::fromSecsSinceEpoch(static_cast<qint64>(backupInfo->ts()))),
    status(convertStatus(backupInfo))
{}

QmlSyncData::QmlSyncData(mega::MegaRequest* request, mega::MegaApi* api)
{
    syncID = request->getParentHandle();
    type = (request->getParamType() == mega::MegaSync::TYPE_TWOWAY) ? QmlSyncType::SYNC :
                                                                      QmlSyncType::BACKUP;
    nodeHandle = request->getNodeHandle();
    localFolder = QString::fromUtf8(request->getFile());
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
        return (sync->getError() == mega::MegaSync::NO_SYNC_ERROR) ? SyncStatus::PAUSED :
                                                                     SyncStatus::STOPPED;
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
    auto syncSetting = SyncInfo::instance()->getSyncSettingByTag(backupInfo->id());
    if (!syncSetting || syncSetting->getError() != mega::MegaSync::NO_SYNC_ERROR)
    {
        return SyncStatus::STOPPED;
    }
    return convertStatus(syncSetting->getSync());
}
