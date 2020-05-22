
#include "Model.h"
#include "platform/Platform.h"

//#include <QDesktopServices>
#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

Model *Model::model = NULL;

Model *Model::instance()
{
    if (!model)
    {
        model = new Model();
    }
    return Model::model;
}


Model::Model() : QObject(), mutex(QMutex::Recursive)
{
    preferences = Preferences::instance();
}

void Model::removeSyncedFolder(int num)
{
    QMutexLocker qm(&mutex);
    auto cs = configuredSyncsMap[configuredSyncs.at(num)];
    if (cs->isActive())
    {
        deactivateSync(cs);
    }

    assert(preferences->logged());
    preferences->removeSyncSetting(cs);
    configuredSyncsMap.remove(configuredSyncs.at(num));
    configuredSyncs.removeAt(num);

    emit syncRemoved(cs);
}

void Model::removeSyncedFolderByTag(int tag)
{
    QMutexLocker qm(&mutex);
    auto cs = configuredSyncsMap[tag];

    if (cs->isActive())
    {
        deactivateSync(cs);
    }
    assert(preferences->logged());

    preferences->removeSyncSetting(cs);
    configuredSyncsMap.remove(tag);

    auto it = configuredSyncs.begin();
    while (it != configuredSyncs.end())
    {
        if ((*it) == tag)
        {
            it = configuredSyncs.erase(it);
        }
        else
        {
            ++it;
        }
    }

    emit syncRemoved(cs);
}

void Model::removeAllFolders()
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged());

    //remove all configured syncs
    preferences->removeAllFolders();


    for (auto it = configuredSyncsMap.begin(); it != configuredSyncsMap.end(); it++)
    {
        //TODO: reuse this one whenevere else syncFolderRemoved needs to be called!
        Platform::syncFolderRemoved(it.value()->getLocalFolder(), it.value()->name(), QString::number(it.value()->tag()));

        //TODO: notifyItemChange?
    }
    configuredSyncs.clear();
    configuredSyncsMap.clear();
}

void Model::activateSync(std::shared_ptr<SyncSetting> syncSetting)
{
    // set sync name if none.
    if (syncSetting->name().isEmpty()) //TODO: find other points of setting name and review consistency
    {
        QFileInfo localFolderInfo(syncSetting->getLocalFolder());
        QString syncName = localFolderInfo.fileName();
        if (syncName.isEmpty())
        {
            syncName = QDir::toNativeSeparators(syncSetting->getLocalFolder());
        }
        syncName.remove(QChar::fromAscii(':')).remove(QDir::separator());
        syncSetting->setName(syncName);
    }

    assert( syncSetting->getLocalFolder() == QDir::toNativeSeparators(QFileInfo(syncSetting->getLocalFolder()).canonicalFilePath()) );

    if (syncSetting->getSyncID().isEmpty())
    {
        syncSetting->setSyncID(QUuid::createUuid().toString().toUpper());
    }

    Platform::syncFolderAdded(syncSetting->getLocalFolder(), syncSetting->name(), syncSetting->getSyncID());
}

void Model::deactivateSync(std::shared_ptr<SyncSetting> syncSetting)
{
    Platform::syncFolderRemoved(syncSetting->getLocalFolder(), syncSetting->name(), syncSetting->getSyncID());
    //TODO: notifyItemChange?

}

std::shared_ptr<SyncSetting> Model::updateSyncSettings(MegaSync *sync, int addingState, const char *remotePath)
{
    if (!sync)
    {
        return nullptr;
    }

    QMutexLocker qm(&mutex);
    assert(preferences->logged());

    std::shared_ptr<SyncSetting> cs;
    bool wasEnabled = true;
    bool wasActive = false;
    bool wasInactive = false;

    if (configuredSyncsMap.contains(sync->getTag())) //existing configuration (an update or a resume after picked from old sync config)
    {
        cs = configuredSyncsMap[sync->getTag()];
        wasEnabled = cs->isEnabled();
        wasActive = cs->isActive();
        wasInactive = !cs->isActive(); //beware: empty MEGAsync's are in INITIAL_SYNC: i.e: active! [problematic for picked configs]
        cs->setSync(sync);

        assert(remotePath || cs->getMegaFolder().size()); //updated syncs should always have a remote path
        if (remotePath)
        {
            cs->setMegaFolder(remotePath);
        }
    }
    else //new configuration (new or resumed)
    {
        assert(addingState && "!addingState and didn't find previously configured sync");

        auto loaded = preferences->getLoadedSyncsMap();
        if (loaded.contains(sync->getTag())) //existing configuration from previous executions (we get the data that the sdk might not be providing from our cache)
        {
            cs = configuredSyncsMap[sync->getTag()] = std::make_shared<SyncSetting>(*loaded[sync->getTag()].get());
            cs->setSync(sync);
            assert(remotePath || cs->getMegaFolder().size()); //updated syncs should always have a remote path
            if (remotePath)
            {
                cs->setMegaFolder(remotePath);
            }
        }
        else // new addition (no reference in the cache)
        {
            cs = configuredSyncsMap[sync->getTag()] = std::make_shared<SyncSetting>(sync, remotePath);
        }

        configuredSyncs.append(sync->getTag());
    }

    if (addingState) //new or resumed
    {
        wasActive = (addingState == MegaSync::SyncAdded::FROM_CACHE && cs->isActive() )
                || addingState == MegaSync::SyncAdded::FROM_CACHE_FAILED_TO_RESUME;

        wasInactive =  (addingState == MegaSync::SyncAdded::FROM_CACHE && !cs->isActive() )
                || addingState == MegaSync::SyncAdded::NEW || addingState == MegaSync::SyncAdded::FROM_CACHE_REENABLED
                || addingState == MegaSync::SyncAdded::REENABLED_FAILED;
    }

    preferences->writeSyncSetting(cs);

    if (cs->isActive() && wasInactive)
    {
        activateSync(cs);
    }

    if (!cs->isActive() && wasActive )
    {
        deactivateSync(cs);
    }

    emit syncStateChanged(cs);
    return cs;
}

void Model::pickInfoFromOldSync(const SyncData &osd, int tag)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged());
    std::shared_ptr<SyncSetting> cs;

    if (!configuredSyncsMap.contains(tag)) //this should always be the case
    {
        cs = configuredSyncsMap[tag] = std::make_shared<SyncSetting>();
    }
    cs->setTag(tag);
    cs->setName(osd.mName);
    cs->setSyncID(osd.mSyncID);
    cs->setEnabled(osd.mEnabled);

    configuredSyncs.append(tag);

    preferences->writeSyncSetting(cs);
}

void Model::reset()
{
    configuredSyncs.clear();
}

int Model::getNumSyncedFolders()
{
    QMutexLocker qm(&mutex);
    return  configuredSyncs.size();
}

QString Model::getSyncName(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size()>num));
    if (num >= configuredSyncs.size())
    {
        return QString();
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->name();
}

QString Model::getSyncID(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size() > num));
    if (num >= configuredSyncs.size())
    {
        return QString();
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->getSyncID();
}

QString Model::getLocalFolder(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size()>num));
    if (num >= configuredSyncs.size())
    {
        return QString();
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->getLocalFolder();
}

QString Model::getMegaFolder(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size()>num)/* && configuredSyncs.at(num).sync()*/); //TODO: add the same check in all similar asserts?
    if (num >= configuredSyncs.size())
    {
        return QString();
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->getMegaFolder();
}

long long Model::getLocalFingerprint(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size()>num));
    if (num >= configuredSyncs.size())
    {
        return 0;
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->getLocalFingerprint();
}

MegaHandle Model::getMegaFolderHandle(int num)
{
    QMutexLocker qm(&mutex);
    assert(preferences->logged() && (configuredSyncs.size()>num));
    if (num >= configuredSyncs.size())
    {
        return mega::INVALID_HANDLE;
    }

    return configuredSyncsMap[configuredSyncs.at(num)]->getMegaHandle();
}

bool Model::isFolderActive(int num) //TODO: study usages of this one
{
    QMutexLocker qm(&mutex);
    if (num >= configuredSyncs.size())
    {
        return false;
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->isEnabled();
}

bool Model::isTemporaryInactiveFolder(int num)
{
    QMutexLocker qm(&mutex);
    if (num >= configuredSyncs.size())
    {
        return false;
    }
    return configuredSyncsMap[configuredSyncs.at(num)]->isTemporaryDisabled();
}

QStringList Model::getSyncNames()
{
    QMutexLocker qm(&mutex);
    QStringList value;
    for (auto &cs : configuredSyncs)
    {
        value.append(configuredSyncsMap[cs]->name());
    }
    return value;
}

QStringList Model::getSyncIDs()
{
    QMutexLocker qm(&mutex);
    QStringList value;
    for (auto &cs : configuredSyncs)
    {
        value.append(QString::number(configuredSyncsMap[cs]->tag()));
    }
    return value;
}

QStringList Model::getMegaFolders()
{
    QMutexLocker qm(&mutex);
    QStringList value;
    for (auto &cs : configuredSyncs)
    {
        value.append(configuredSyncsMap[cs]->getMegaFolder());
    }
    return value;
}

QStringList Model::getLocalFolders()
{
    QMutexLocker qm(&mutex);
    QStringList value;
    for (auto &cs : configuredSyncs)
    {
        value.append(configuredSyncsMap[cs]->getLocalFolder());
    }
    return value;
}

QList<long long> Model::getMegaFolderHandles()
{
    QMutexLocker qm(&mutex);
    QList<long long> value;
    for (auto &cs : configuredSyncs)
    {
        value.append(configuredSyncsMap[cs]->getMegaHandle());
    }
    return value;
}

std::shared_ptr<SyncSetting> Model::getSyncSetting(int num)
{
    QMutexLocker qm(&mutex);
    return configuredSyncsMap[configuredSyncs.at(num)];
}


std::shared_ptr<SyncSetting> Model::getSyncSettingByTag(int tag)
{
    QMutexLocker qm(&mutex);
    if (configuredSyncsMap.contains(tag))
    {
        return configuredSyncsMap[tag];
    }
    return nullptr;
}

