
#include "SyncInfo.h"
#include "platform/Platform.h"
#include "control/AppStatsEvents.h"
#include "QMegaMessageBox.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include <MegaNodeNames.h>
#include <mega/types.h>

#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const QVector<SyncInfo::SyncType> SyncInfo::AllHandledSyncTypes =
{
    SyncInfo::SyncType::TYPE_TWOWAY,
    SyncInfo::SyncType::TYPE_BACKUP,
};

std::unique_ptr<SyncInfo> SyncInfo::model;

SyncInfo *SyncInfo::instance()
{
    if (!model)
    {
        model.reset(new SyncInfo());
    }
    return SyncInfo::model.get();
}

SyncInfo::SyncInfo() : QObject(),
    preferences (Preferences::instance()),
    mIsFirstTwoWaySyncDone (preferences->isFirstSyncDone()),
    mIsFirstBackupDone (preferences->isFirstBackupDone()),
    syncMutex (QMutex::Recursive),
    delegateListener(mega::make_unique<QTMegaListener>(MegaSyncApp->getMegaApi(), this))
{
    MegaSyncApp->getMegaApi()->addListener(delegateListener.get());

    mShowErrorTimer.setSingleShot(true);
    mShowErrorTimer.setInterval(500);
    connect(&mShowErrorTimer, &QTimer::timeout, this, &SyncInfo::checkUnattendedDisabledSyncsForErrors);
}

bool SyncInfo::hasUnattendedDisabledSyncs(const QVector<SyncType>& types) const
{
    return std::any_of(types.cbegin(), types.cend(), [this](SyncType t){return !unattendedDisabledSyncs[t].isEmpty();});
}

const QSet<MegaHandle> SyncInfo::getUnattendedDisabledSyncs(const SyncType &type) const
{
    return unattendedDisabledSyncs[type];
}

void SyncInfo::removeSyncedFolderByBackupId(MegaHandle backupId)
{
    QMutexLocker qm(&syncMutex);

    auto cs = configuredSyncsMap.value(backupId, nullptr);

    if (!cs)
    {
        return;
    }

    if(cs->isActive())
    {
        deactivateSync(cs);
    }

    Platform::getInstance()->syncFolderRemoved(cs->getLocalFolder(), cs->name(true), cs->getSyncID());

    assert(preferences->logged());

    preferences->removeSyncSetting(cs);
    configuredSyncsMap.remove(backupId);

    auto type (cs->getType());

    auto it = configuredSyncs[type].begin();
    while (it != configuredSyncs[type].end())
    {
        if ((*it) == backupId)
        {
            it = configuredSyncs[type].erase(it);
        }
        else
        {
            ++it;
        }
    }

    removeUnattendedDisabledSync(backupId, type);

    emit syncRemoved(cs);
}

void SyncInfo::removeAllFolders()
{
    QMutexLocker qm(&syncMutex);
    assert(preferences->logged());

    //remove all configured syncs
    preferences->removeAllFolders();

    for (auto it = configuredSyncsMap.begin(); it != configuredSyncsMap.end(); it++)
    {
        if (it.value()->isActive())
        {
            deactivateSync(it.value());
        }
    }
    configuredSyncs.clear();
    configuredSyncsMap.clear();
    syncsSettingPickedFromOldConfig.clear();
    unattendedDisabledSyncs.clear();
}

void SyncInfo::activateSync(std::shared_ptr<SyncSettings> syncSetting)
{
    // set sync UID
    if (syncSetting->getSyncID().isEmpty())
    {
        syncSetting->setSyncID(QUuid::createUuid().toString().toUpper());
        Platform::getInstance()->syncFolderAdded(syncSetting->getLocalFolder(), syncSetting->name(true), syncSetting->getSyncID());
    }

    //send event for the first sync/backup
    switch (syncSetting->getType())
    {
    case mega::MegaSync::SyncType::TYPE_TWOWAY:
    {
        // Send event for the first sync
        if (!mIsFirstTwoWaySyncDone && !preferences->isFirstSyncDone())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_1ST_SYNC,
                                                 "MEGAsync first sync", false, nullptr);
        }
        mIsFirstTwoWaySyncDone = true;
        break;
    }
    case mega::MegaSync::SyncType::TYPE_BACKUP:
    {
        // Send event for the first backup
        if (!mIsFirstBackupDone && !preferences->isFirstBackupDone())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_1ST_BACKUP,
                                                 "MEGAsync first backup", false, nullptr);
        }
        mIsFirstBackupDone = true;
        break;
    }
    default:
        break;
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title =  MegaSyncApp->getMEGAString();

    // TODO: extract the QMegaMessageBoxes from the model, use signal to send message
    if (!preferences->isFatWarningShown() && syncSetting->getError() == MegaSync::Warning::LOCAL_IS_FAT)
    {
        msgInfo.text = tr("You are syncing a local folder formatted with a FAT filesystem. "
                          "That filesystem has deficiencies managing big files and modification"
                          " times that can cause synchronization problems (e.g. when daylight "
                          "saving changes), so it's strongly recommended that you only sync "
                          "folders formatted with more reliable filesystems like NTFS (more information [A]here[/A]).")
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"https://help.mega.nz/megasync/syncing.html#can-i-sync-fat-fat32-partitions-under-windows\">"))
                .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</a>"));
        msgInfo.finishFunc = [this](QPointer<QMessageBox>)
        {
            preferences->setFatWarningShown();
        };

        QMegaMessageBox::warning(msgInfo);
    }
    else if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING) && syncSetting->getError() == MegaSync::Warning::LOCAL_IS_HGFS)
    {
        msgInfo.text = tr("You are syncing a local folder shared with VMWare. Those folders do not support filesystem notifications so MEGAsync will have to be continuously scanning to detect changes in your files and folders. Please use a different folder if possible to reduce the CPU usage.");
        msgInfo.finishFunc = [this](QPointer<QMessageBox>)
        {
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING, true);
        };
        QMegaMessageBox::warning(msgInfo);
    }
}

void SyncInfo::deactivateSync(std::shared_ptr<SyncSettings> syncSetting)
{
    Platform::getInstance()->notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
}

void SyncInfo::updateMegaFolder(QString newRemotePath, std::shared_ptr<SyncSettings> cs)
{
    QMutexLocker qm(&syncMutex);
    auto oldMegaFolder = cs->getMegaFolder();
    cs->setMegaFolder(newRemotePath);
    if (oldMegaFolder != newRemotePath)
    {
        Utilities::queueFunctionInAppThread([=]() //we need this for emit to work!
        {//queued function
            emit syncStateChanged(cs);
        });//end of queued function
    }
}

std::shared_ptr<SyncSettings> SyncInfo::updateSyncSettings(MegaSync *sync)
{
    if (!sync)
    {
        return nullptr;
    }

    QMutexLocker qm(&syncMutex);

    std::shared_ptr<SyncSettings> cs;
    bool wasActive = false;
    bool wasInactive = false;

    auto oldcsitr = syncsSettingPickedFromOldConfig.find(sync->getBackupId());

    if (oldcsitr != syncsSettingPickedFromOldConfig.end()) // resumed after picked from old sync config)
    {
        cs = oldcsitr.value();

        //move into the configuredSyncsMap
        configuredSyncsMap.insert(sync->getBackupId(), cs);
        MegaSync::SyncType type = static_cast<MegaSync::SyncType>(sync->getType());
        configuredSyncs[type].append(sync->getBackupId());

        // remove from picked
        syncsSettingPickedFromOldConfig.erase(oldcsitr);
    }
    else if (configuredSyncsMap.contains(sync->getBackupId())) //existing configuration (an update)
    {
        cs = configuredSyncsMap[sync->getBackupId()];
    }

    if (cs)
    {
        wasActive = cs->isActive();
        wasInactive = !wasActive;

        cs->setSync(sync);
    }
    else //new configuration (new or resumed)
    {
        auto loaded = preferences->getLoadedSyncsMap();
        if (loaded.contains(sync->getBackupId())) //existing configuration from previous executions (we get the data that the sdk might not be providing from our cache)
        {
            cs = configuredSyncsMap[sync->getBackupId()] = std::make_shared<SyncSettings>(*loaded[sync->getBackupId()].get());
            cs->setSync(sync);
        }
        else // new addition (no reference in the cache)
        {
            cs = configuredSyncsMap[sync->getBackupId()] = std::make_shared<SyncSettings>(sync);
        }

        configuredSyncs[static_cast<SyncType>(sync->getType())].append(sync->getBackupId());
    }

    //queue an update of the sync remote node
    ThreadPoolSingleton::getInstance()->push([this, cs]()
    {//thread pool function

        std::unique_ptr<char[]> np(MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(cs->getMegaHandle()));
        updateMegaFolder(np ? QString::fromUtf8(np.get()) : QString(), cs);

    });// end of thread pool function

    if (cs->isActive() && wasInactive)
    {
        activateSync(cs);
    }

    if (!cs->isActive() && wasActive)
    {
        deactivateSync(cs);
    }

    preferences->writeSyncSetting(cs); // we store MEGAsync specific fields into cache

    emit syncStateChanged(cs);
    return cs;
}

void SyncInfo::rewriteSyncSettings()
{
    preferences->removeAllSyncSettings();
    QMutexLocker qm(&syncMutex);

    // Get all settings
    const auto listAllSyncs (configuredSyncs.values());
    const auto settings (std::accumulate(listAllSyncs.begin(),
                                         listAllSyncs.end(),
                                         QList<mega::MegaHandle>()));
    for (auto cs : settings)
    {
        preferences->writeSyncSetting(configuredSyncsMap[cs]); // we store MEGAsync specific fields into cache
    }
}

void SyncInfo::pickInfoFromOldSync(const SyncData &osd, MegaHandle backupId, bool loadedFromPreviousSessions)
{
    QMutexLocker qm(&syncMutex);
    assert(preferences->logged() || loadedFromPreviousSessions);
    assert (!configuredSyncsMap.contains(backupId) && "picking already configured sync!"); //this should always be the case

    std::shared_ptr<SyncSettings> cs = syncsSettingPickedFromOldConfig[backupId] = std::make_shared<SyncSettings>(osd, loadedFromPreviousSessions);

    cs->setBackupId(backupId); //assign the new tag given by the sdk

    preferences->writeSyncSetting(cs);
}

void SyncInfo::reset()
{
    QMutexLocker qm(&syncMutex);
    configuredSyncs.clear();
    configuredSyncsMap.clear();
    syncsSettingPickedFromOldConfig.clear();
    unattendedDisabledSyncs.clear();
    mIsFirstTwoWaySyncDone = false;
    mIsFirstBackupDone = false;
}

int SyncInfo::getNumSyncedFolders(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    int value (0);
    for (auto type : types)
    {
        value += configuredSyncs[type].size();
    }
    return value;
}

bool SyncInfo::syncWithErrorExist(const QVector<SyncType> &types)
{
    QMutexLocker qm(&syncMutex);
    for(auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            if(configuredSyncsMap[cs]->getError())
            {
                return true;
            }
        }
    }
    return false;
}

QStringList SyncInfo::getSyncNames(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            value.append(configuredSyncsMap[cs]->name());
        }
    }
    return value;
}

QStringList SyncInfo::getSyncIDs(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            value.append(QString::number(configuredSyncsMap[cs]->backupId()));
        }
    }
    return value;
}

QStringList SyncInfo::getCloudDriveSyncMegaFolders(bool cloudDrive)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    for (auto type : AllHandledSyncTypes)
    {
        for (auto &cs : configuredSyncs[type])
        {
            QString megaFolder = configuredSyncsMap[cs]->getMegaFolder();
            auto parent_node = std::unique_ptr<MegaNode>(megaApi->getNodeByPath(megaFolder.toStdString().data()));

            while(parent_node && parent_node->getParentHandle() != INVALID_HANDLE)
            {
                parent_node = std::unique_ptr<MegaNode>(megaApi->getNodeByHandle(parent_node->getParentHandle()));
            }

            if(!parent_node)
            {
                continue;
            }

            if((parent_node->isInShare() && !cloudDrive)
                    || (megaApi->getRootNode()->getHandle() == parent_node->getHandle() && cloudDrive))
            {
                value.append(megaFolder.append(QLatin1Char('/')));
            }
        }
    }
    return value;
}

QSet<QString> SyncInfo::getRemoteBackupFolderNames()
{
    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    QSet<QString> backupsNames;

    if (myBackupsHandle->getMyBackupsHandle() != INVALID_HANDLE)
    {
        auto api (MegaSyncApp->getMegaApi());
        std::unique_ptr<MegaNode> myBackupsNode (api->getNodeByHandle(myBackupsHandle->getMyBackupsHandle()));
        std::unique_ptr<const char[]> deviceIdRaw (api->getDeviceId());
        QString deviceId (QString::fromLatin1(deviceIdRaw.get()));

        std::unique_ptr<MegaNodeList> devices (api->getChildren(myBackupsNode.get()));
        int i = 0;
        MegaNode* deviceNode (nullptr);

        while (!deviceNode && devices && i < devices->size())
        {
            if (QString::fromLatin1(devices->get(i)->getDeviceId()) == deviceId)
            {
                deviceNode = devices->get(i);
            }
            i++;
        }

        if (deviceNode)
        {
            std::unique_ptr<MegaNodeList> folders (api->getChildren(deviceNode));
            for (int j = 0; folders && j < folders->size(); j++)
            {
                backupsNames.insert(QString::fromUtf8(folders->get(j)->getName()));
            }
        }
    }
    return backupsNames;
}

QStringList SyncInfo::getMegaFolders(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            value.append(configuredSyncsMap[cs]->getMegaFolder());
        }
    }
    return value;
}

QStringList SyncInfo::getLocalFolders(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;

    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            value.append(configuredSyncsMap[cs]->getLocalFolder());
        }
    }
    return value;
}

QMap<QString, SyncInfo::SyncType> SyncInfo::getLocalFoldersAndTypeMap(bool normalized)
{
    QMutexLocker qm(&syncMutex);
    QMap<QString, SyncType> ret;

    for (auto type : SyncInfo::AllHandledSyncTypes)
    {
        for (auto &cs : configuredSyncs[type])
        {
            ret.insert(configuredSyncsMap[cs]->getLocalFolder(normalized), type);
        }
    }
    return ret;
}

QList<MegaHandle> SyncInfo::getMegaFolderHandles(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QList<MegaHandle> value;
    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            value.append(configuredSyncsMap[cs]->getMegaHandle());
        }
    }
    return value;
}

std::shared_ptr<SyncSettings> SyncInfo::getSyncSetting(int num, mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    return configuredSyncsMap[configuredSyncs[type].at(num)];
}

QList<std::shared_ptr<SyncSettings>> SyncInfo::getSyncSettingsByType(const QVector<SyncType>& types)
{
    QMutexLocker qm(&syncMutex);
    QList<std::shared_ptr<SyncSettings>> syncs;
    for (auto type : types)
    {
        for (auto &cs : configuredSyncs[type])
        {
            syncs.append(configuredSyncsMap[cs]);
        }
    }
    return syncs;
}

std::shared_ptr<SyncSettings> SyncInfo::getSyncSettingByTag(MegaHandle tag) const
{
    QMutexLocker qm(&syncMutex);

    return configuredSyncsMap.value(tag, nullptr);
}

void SyncInfo::saveUnattendedDisabledSyncs()
{
    if (preferences->logged())
    {
        const auto allTags (unattendedDisabledSyncs.values());
        const auto tags (std::accumulate(allTags.begin(),
                                             allTags.end(),
                                             QSet<mega::MegaHandle>()));
        preferences->setDisabledSyncTags(tags);
    }
}

void SyncInfo::checkUnattendedDisabledSyncsForErrors()
{
    auto syncsUnattended = getUnattendedDisabledSyncs(MegaSync::TYPE_TWOWAY);
    auto backupsUnattended = getUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP);

    if ((syncsUnattended.size() + backupsUnattended.size()) == 1)
    {
        if (syncsUnattended.size() == 1)
        {
            showSingleSyncDisabledNotification(getSyncSettingByTag(*syncsUnattended.begin()));
        }
        else if (backupsUnattended.size() == 1)
        {
            showSingleSyncDisabledNotification(getSyncSettingByTag(*backupsUnattended.begin()));
        }
    }
    else if (!syncsUnattended.isEmpty() || !backupsUnattended.isEmpty())
    {
        QString megaSyncError(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(mLastError)));

        if (!syncsUnattended.isEmpty() &&
            !backupsUnattended.isEmpty())
        {
            MegaSyncApp->showErrorMessage(tr("Your syncs and backups have been disabled: %1").arg(megaSyncError));
        }
        else if (!backupsUnattended.isEmpty())
        {
            MegaSyncApp->showErrorMessage(tr("Your backups have been disabled: %1").arg(megaSyncError));
        }
        else
        {
            MegaSyncApp->showErrorMessage(tr("Your syncs have been disabled: %1").arg(megaSyncError));
        }

        mLastError = mega::MegaSync::NO_SYNC_ERROR;
    }
}

void SyncInfo::addUnattendedDisabledSync(MegaHandle tag, mega::MegaSync::SyncType type)
{
    if(unattendedDisabledSyncs.contains(type))
    {
        unattendedDisabledSyncs[type].insert(tag);
        saveUnattendedDisabledSyncs();
        emit syncDisabledListUpdated();

        mShowErrorTimer.start();
    }
}

void SyncInfo::removeUnattendedDisabledSync(MegaHandle tag, mega::MegaSync::SyncType type)
{
    if(unattendedDisabledSyncs.contains(type))
    {
        unattendedDisabledSyncs[type].remove(tag);
        saveUnattendedDisabledSyncs();
        emit syncDisabledListUpdated();
    }
}

void SyncInfo::setUnattendedDisabledSyncs(const QSet<MegaHandle>& tags)
{
    for (auto tag : tags)
    {
        auto sync (configuredSyncsMap[tag]);
        if (sync)
        {
            unattendedDisabledSyncs[sync->getType()].insert(tag);
        }
    }
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void SyncInfo::dismissUnattendedDisabledSyncs(const QVector<SyncType>& types)
{
    for (auto type : types)
    {
        unattendedDisabledSyncs[type].clear();
    }
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void SyncInfo::onEvent(mega::MegaApi*, mega::MegaEvent* event)
{
    if (event->getType() == MegaEvent::EVENT_SYNCS_RESTORED)
	{
		if (getNumSyncedFolders(SyncInfo::AllHandledSyncTypes) > 0)
		{
			Platform::getInstance()->notifyAllSyncFoldersAdded();
		}
    }
}

void SyncInfo::onSyncStateChanged(mega::MegaApi*, mega::MegaSync *sync)
{
    auto syncSettings = getSyncSettingByTag(sync->getBackupId());
    if (!syncSettings)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("onSyncDisabled for non existing sync").toUtf8().constData());
        return;
    }

    if(!syncSettings->isActive() && sync->getRunState() == MegaSync::RUNSTATE_DISABLED)
    {
        return;
    }

    updateSyncSettings(sync);

    if(sync->getError() != mega::MegaError::API_OK)
    {
        mLastError = sync->getError();

        if (sync->getRunState() == MegaSync::RUNSTATE_DISABLED)
        {
            addUnattendedDisabledSync(sync->getBackupId(),
                                      static_cast<MegaSync::SyncType>(sync->getType()));
        }
    }
    else
    {
        if (sync->getRunState() != MegaSync::RUNSTATE_DISABLED)
        {
            removeUnattendedDisabledSync(sync->getBackupId(),
                                         static_cast<MegaSync::SyncType>(sync->getType()));
        }
    }
}

void SyncInfo::onSyncDeleted(MegaApi*, MegaSync *sync)
{
    removeSyncedFolderByBackupId(sync->getBackupId());
}

void SyncInfo::onSyncAdded(MegaApi*, MegaSync *sync)
{
    updateSyncSettings(sync);
}

void SyncInfo::onSyncFileStateChanged(MegaApi*, MegaSync*, std::string *localPath, int newState)
{
    if (!localPath || localPath->empty())
    {
        return;
    }

    Platform::getInstance()->notifySyncFileChange(localPath, newState);
}

void SyncInfo::showSingleSyncDisabledNotification(std::shared_ptr<SyncSettings> syncSetting)
{
    if (syncSetting)
    {
        auto errorCode(syncSetting->getError());
        auto syncType(syncSetting->getType());
        QString syncName(syncSetting->name());

        if (syncType == MegaSync::TYPE_TWOWAY)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Sync \"%1\" Path: %2 disabled: %3")
                .arg(syncName, syncSetting->getLocalFolder(), QString::number(errorCode)).toUtf8().constData());

            if (errorCode != MegaSync::NO_SYNC_ERROR
                && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                switch (errorCode)
                {
                case MegaSync::Error::NO_SYNC_ERROR:
                {
                    assert(false && "unexpected no error after onSyncAdded failed");
                    return;
                }
                case MegaSync::Error::LOCAL_PATH_UNAVAILABLE:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::REMOTE_NODE_NOT_FOUND:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::VBOXSHAREDFOLDER_UNSUPPORTED:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::REMOTE_NODE_MOVED_TO_RUBBISH:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::SHARE_NON_FULL_ACCESS:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::LOCAL_FILESYSTEM_MISMATCH:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder has changed")
                        .arg(syncName));
                    break;
                }
                case MegaSync::Error::PUT_NODES_ERROR:
                default:
                {
                    MegaSyncApp->showErrorMessage(tr("Your sync \"%1\" has been disabled. Reason: %2").arg(syncName,
                        QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                    break;
                }
                }
            }
        }
        else if (syncType == MegaSync::TYPE_BACKUP)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Backup \"%1\" Path: %2 disabled: %3")
                .arg(syncName, syncSetting->getLocalFolder(), QString::number(errorCode)).toUtf8().constData());
            if (!syncSetting->isEnabled()
                && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                QString errMsg(tr("Your backup \"%1\" has been temporarily disabled: %2")
                    .arg(syncName, QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                MegaSyncApp->showErrorMessage(errMsg);
            }
			else if (errorCode != MegaSync::NO_SYNC_ERROR
				&& errorCode != MegaSync::Error::LOGGED_OUT)
			{
				switch (errorCode)
				{
				case MegaSync::Error::NO_SYNC_ERROR:
				{
					assert(false && "unexpected no error after onSyncAdded failed");
					return;
				}
				case MegaSync::Error::LOCAL_PATH_UNAVAILABLE:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled because the local folder doesn't exist")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::REMOTE_NODE_NOT_FOUND:
				{
					// We don't want to show a notification here because the removal of the remote
					// folder means that the backup has been deleted from the Backups Center
					break;
				}
				case MegaSync::Error::VBOXSHAREDFOLDER_UNSUPPORTED:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::REMOTE_NODE_MOVED_TO_RUBBISH:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled because the remote folder is in the rubbish bin")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::SHARE_NON_FULL_ACCESS:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::LOCAL_FILESYSTEM_MISMATCH:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled because the local folder has changed")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::BACKUP_MODIFIED:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled because the remote folder has changed")
						.arg(syncName));
					break;
				}
				case MegaSync::Error::PUT_NODES_ERROR:
				default:
				{
					MegaSyncApp->showErrorMessage(tr("Your backup \"%1\" has been disabled. Reason: %2").arg(syncName,
						QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
					break;
				}
				}
			}
		}
		else
		{
			MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
				QString::fromLatin1("Unknown type of sync: %1")
				.arg(syncType).toUtf8().constData());
		}
	}
}
