#include "Model.h"
#include "platform/Platform.h"
#include "control/AppStatsEvents.h"

#include "QMegaMessageBox.h"
#include <QHostInfo>

#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

std::unique_ptr<Model> Model::model;

Model *Model::instance()
{
    if (!model)
    {
        model.reset(new Model());
    }
    return Model::model.get();
}

Model::Model() : QObject(),
    preferences (Preferences::instance()),
    mDeviceName(),
    mBackupsDirHandle(mega::INVALID_HANDLE),
    syncMutex (QMutex::Recursive)
{
}

bool Model::hasUnattendedDisabledSyncs() const
{
    return unattendedDisabledSyncs.size();
}

void Model::removeSyncedFolder(int num, mega::MegaSync::SyncType type)
{
    assert(num <= configuredSyncs[type].size()
           && configuredSyncsMap.contains(configuredSyncs[type].at(num)));
    QMutexLocker qm(&syncMutex);
    auto cs = configuredSyncsMap[configuredSyncs[type].at(num)];
    if (cs->isActive())
    {
        deactivateSync(cs);
    }

    auto backupId = cs->backupId();

    assert(preferences->logged());
    preferences->removeSyncSetting(cs);
    configuredSyncsMap.remove(configuredSyncs[type].at(num));
    configuredSyncs[type].removeAt(num);

    removeUnattendedDisabledSync(backupId);

    emit syncRemoved(cs);
}

void Model::removeSyncedFolderByBackupId(MegaHandle backupId)
{
    QMutexLocker qm(&syncMutex);
    if (!configuredSyncsMap.contains(backupId))
    {
        return;
    }

    auto cs = configuredSyncsMap[backupId];

    if (cs->isActive())
    {
        deactivateSync(cs);
    }
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

    removeUnattendedDisabledSync(backupId);

    emit syncRemoved(cs);
}

void Model::removeAllFolders()
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

void Model::activateSync(std::shared_ptr<SyncSetting> syncSetting)
{
#ifndef NDEBUG
    {
        auto sl = syncSetting->getLocalFolder();
#ifdef _WIN32
        if (sl.startsWith(QString::fromAscii("\\\\?\\")))
        {
            sl = sl.mid(4);
        }
#endif
        auto slc = QDir::toNativeSeparators(QFileInfo(sl).canonicalFilePath());
        assert(sl == slc);
    }
#endif

    // set sync UID
    if (syncSetting->getSyncID().isEmpty())
    {
        syncSetting->setSyncID(QUuid::createUuid().toString().toUpper());
    }

    //send event for the first sync
    if (!isFirstSyncDone && !preferences->isFirstSyncDone())
    {
        MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_1ST_SYNC,
                                             "MEGAsync first sync");
    }
    isFirstSyncDone = true;

    if ( !preferences->isFatWarningShown() && syncSetting->getError() == MegaSync::Warning::LOCAL_IS_FAT)
    {
        QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
         tr("You are syncing a local folder formatted with a FAT filesystem. That filesystem has deficiencies managing big files and modification times that can cause synchronization problems (e.g. when daylight saving changes), so it's strongly recommended that you only sync folders formatted with more reliable filesystems like NTFS (more information [A]here[/A]).")
         .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"https://help.mega.nz/megasync/syncing.html#can-i-sync-fat-fat32-partitions-under-windows\">"))
         .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</a>")));
        preferences->setFatWarningShown();
    }
    else if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING) && syncSetting->getError() == MegaSync::Warning::LOCAL_IS_HGFS)
    {
        QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
            tr("You are syncing a local folder shared with VMWare. Those folders do not support filesystem notifications so MEGAsync will have to be continuously scanning to detect changes in your files and folders. Please use a different folder if possible to reduce the CPU usage."));
        preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING, true);
    }

    Platform::syncFolderAdded(syncSetting->getLocalFolder(), syncSetting->name(true), syncSetting->getSyncID());
}

void Model::deactivateSync(std::shared_ptr<SyncSetting> syncSetting)
{
    Platform::syncFolderRemoved(syncSetting->getLocalFolder(), syncSetting->name(true), syncSetting->getSyncID());
    MegaSyncApp->notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
}

void Model::updateMegaFolder(QString newRemotePath, std::shared_ptr<SyncSetting> cs)
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

std::shared_ptr<SyncSetting> Model::updateSyncSettings(MegaSync *sync, int addingState)
{
    if (!sync)
    {
        return nullptr;
    }

    QMutexLocker qm(&syncMutex);

    std::shared_ptr<SyncSetting> cs;
    bool wasActive = false;
    bool wasInactive = false;

    auto oldcsitr = syncsSettingPickedFromOldConfig.find(sync->getBackupId());

    if (oldcsitr != syncsSettingPickedFromOldConfig.end()) // resumed after picked from old sync config)
    {
        cs = oldcsitr.value();

        //move into the configuredSyncsMap
        configuredSyncsMap.insert(sync->getBackupId(), cs);
        configuredSyncs[cs->getType()].append(sync->getBackupId());

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
        wasInactive = !cs->isActive();

        cs->setSync(sync);
    }
    else //new configuration (new or resumed)
    {
        assert(addingState && "!addingState and didn't find previously configured sync");

        auto loaded = preferences->getLoadedSyncsMap();
        if (loaded.contains(sync->getBackupId())) //existing configuration from previous executions (we get the data that the sdk might not be providing from our cache)
        {
            cs = configuredSyncsMap[sync->getBackupId()] = std::make_shared<SyncSetting>(*loaded[sync->getBackupId()].get());
            cs->setSync(sync);
        }
        else // new addition (no reference in the cache)
        {
            cs = configuredSyncsMap[sync->getBackupId()] = std::make_shared<SyncSetting>(sync);
        }

        configuredSyncs[static_cast<MegaSync::SyncType>(sync->getType())].append(sync->getBackupId());
    }

    //queue an update of the sync remote node
    ThreadPoolSingleton::getInstance()->push([this, cs]()
    {//thread pool function

        std::unique_ptr<char[]> np(MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(cs->getMegaHandle()));
        updateMegaFolder(np ? QString::fromUtf8(np.get()) : QString(), cs);

    });// end of thread pool function


    if (addingState) //new or resumed
    {
        wasActive = (addingState == MegaSync::SyncAdded::FROM_CACHE && cs->isActive() )
                || addingState == MegaSync::SyncAdded::FROM_CACHE_FAILED_TO_RESUME;

        wasInactive =  (addingState == MegaSync::SyncAdded::FROM_CACHE && !cs->isActive() )
                || addingState == MegaSync::SyncAdded::NEW || addingState == MegaSync::SyncAdded::FROM_CACHE_REENABLED
                || addingState == MegaSync::SyncAdded::REENABLED_FAILED;
    }

    if (cs->isActive() && wasInactive)
    {
        activateSync(cs);
    }

    if (!cs->isActive() && wasActive )
    {
        deactivateSync(cs);
    }

    preferences->writeSyncSetting(cs); // we store MEGAsync specific fields into cache

#ifdef WIN32
    // handle transition from MEGAsync <= 3.0.1.
    // if resumed from cache and the previous version did not have left pane icons, add them
    if (MegaSyncApp->getPrevVersion() && MegaSyncApp->getPrevVersion() <= 3001
            && !preferences->leftPaneIconsDisabled()
            && addingState == MegaSync::SyncAdded::FROM_CACHE && cs->isActive())
    {
        Platform::addSyncToLeftPane(cs->getLocalFolder(), cs->name(), cs->getSyncID());
    }
#endif

    emit syncStateChanged(cs);
    return cs;
}

void Model::rewriteSyncSettings()
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

void Model::pickInfoFromOldSync(const SyncData &osd, MegaHandle backupId, bool loadedFromPreviousSessions)
{
    QMutexLocker qm(&syncMutex);
    assert(preferences->logged() || loadedFromPreviousSessions);
    std::shared_ptr<SyncSetting> cs;

    assert (!configuredSyncsMap.contains(backupId) && "picking already configured sync!"); //this should always be the case

    cs = syncsSettingPickedFromOldConfig[backupId] = std::make_shared<SyncSetting>(osd, loadedFromPreviousSessions);

    cs->setBackupId(backupId); //assign the new tag given by the sdk

    preferences->writeSyncSetting(cs);
}

void Model::reset()
{
    QMutexLocker qm(&syncMutex);
    configuredSyncs.clear();
    configuredSyncsMap.clear();
    syncsSettingPickedFromOldConfig.clear();
    unattendedDisabledSyncs.clear();
    mDeviceName = QString();
    mBackupsDirHandle = mega::INVALID_HANDLE;
    isFirstSyncDone = false;
}

int Model::getNumSyncedFolders(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    return configuredSyncs[type].size();
}

QStringList Model::getSyncNames(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto &cs : configuredSyncs[type])
    {
        value.append(configuredSyncsMap[cs]->name());
    }
    return value;
}

QStringList Model::getSyncIDs(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto &cs : configuredSyncs[type])
    {
        value.append(QString::number(configuredSyncsMap[cs]->backupId()));
    }
    return value;
}

QStringList Model::getMegaFolders(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto &cs : configuredSyncs[type])
    {
        value.append(configuredSyncsMap[cs]->getMegaFolder());
    }
    return value;
}

QStringList Model::getLocalFolders(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    QStringList value;
    for (auto &cs : configuredSyncs[type])
    {
        value.append(configuredSyncsMap[cs]->getLocalFolder());
    }
    return value;
}

QList<MegaHandle> Model::getMegaFolderHandles(mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    QList<MegaHandle> value;
    for (auto &cs : configuredSyncs[type])
    {
        value.append(configuredSyncsMap[cs]->getMegaHandle());
    }
    return value;
}

std::shared_ptr<SyncSetting> Model::getSyncSetting(int num, mega::MegaSync::SyncType type)
{
    QMutexLocker qm(&syncMutex);
    return configuredSyncsMap[configuredSyncs[type].at(num)];
}

QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> Model::getCopyOfSettings()
{
    QMutexLocker qm(&syncMutex);
    return configuredSyncsMap;
}


std::shared_ptr<SyncSetting> Model::getSyncSettingByTag(MegaHandle tag)
{
    QMutexLocker qm(&syncMutex);
    if (configuredSyncsMap.contains(tag))
    {
        return configuredSyncsMap[tag];
    }
    return nullptr;
}

void Model::saveUnattendedDisabledSyncs()
{
    if (preferences->logged())
    {
        preferences->setDisabledSyncTags(unattendedDisabledSyncs);
    }
}

void Model::addUnattendedDisabledSync(MegaHandle tag)
{
    unattendedDisabledSyncs.insert(tag);
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void Model::removeUnattendedDisabledSync(MegaHandle tag)
{
    unattendedDisabledSyncs.remove(tag);
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void Model::setUnattendedDisabledSyncs(QSet<MegaHandle> tags)
{
    //REVIEW: If possible to get enable/disable callbacks before loading from settings.Merge both lists of tags.
    unattendedDisabledSyncs = tags;
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void Model::dismissUnattendedDisabledSyncs()
{
    unattendedDisabledSyncs.clear();
    saveUnattendedDisabledSyncs();
    emit syncDisabledListUpdated();
}

void Model::setDeviceName(const QString& name, bool setRemote)
{
    if (name != mDeviceName)
    {
        mDeviceName = name;

        if (setRemote)
        {
            auto api (MegaSyncApp->getMegaApi());
            assert(preferences->logged() && api);

            mega::SynchronousRequestListener synchro;
            api->setDeviceName(mDeviceName.toUtf8().constData(), &synchro);
            synchro.wait();
        }

        MegaSyncApp->createAppMenus();
    }
}

const QString& Model::getDeviceName()
{
    // If we don't have a device name, try to get it
    if (mDeviceName.isEmpty())
    {
        auto api (MegaSyncApp->getMegaApi());
        assert(preferences->logged() && api);

        mega::SynchronousRequestListener synchro;
        api->getDeviceName(&synchro);
        synchro.wait();

        if (synchro.getError()->getErrorCode() == mega::MegaError::API_OK)
        {
            // The remote dir shoud exist, get name.
            setDeviceName(QString::fromUtf8(synchro.getRequest()->getName()));
        }
        else
        {
            // If we still don't have one, use hostname from the OS
            QString name = QHostInfo::localHostName();

            // If nothing, use generic one.
            if (name.isNull())
            {
                name = tr("Your computer");
            }
            setDeviceName(name, true);
        }
    }

    return mDeviceName;
}

void Model::setBackupsDirHandle(mega::MegaHandle handle)
{
    mBackupsDirHandle = handle;
}

mega::MegaHandle Model::getBackupsDirHandle()
{
    auto api (MegaSyncApp->getMegaApi());
    mega::MegaHandle handle (mBackupsDirHandle);

    // If not set, try to get (synchronously)
    if (handle == mega::INVALID_HANDLE)
    {
        mega::SynchronousRequestListener synchro;
        MegaSyncApp->getMegaApi()->getMyBackupsFolder(&synchro);
        synchro.wait();

        if (synchro.getError()->getErrorCode() == mega::MegaError::API_OK)
        {
            // The remote dir shoud exist, get handle.
            handle = synchro.getRequest()->getNodeHandle();
        }
    }

    // We know the target dir: check existence and if it has not been put in the rubbish bin.
    if (handle != mega::INVALID_HANDLE)
    {
        std::unique_ptr<mega::MegaNode> backupsDirNode (api->getNodeByHandle(handle));
        if (!backupsDirNode || backupsDirNode->getRestoreHandle() != mega::INVALID_HANDLE)
        {
            // Dir does not exist or has been put in RubbishBin
            handle = mega::INVALID_HANDLE;
        }
    }

    // Update member if needed
    if (handle != mBackupsDirHandle)
    {
        setBackupsDirHandle(handle);
    }

    return mBackupsDirHandle;
}
