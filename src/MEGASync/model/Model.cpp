
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
    mutex.lock();
    assert(preferences->logged());
    preferences->removeSyncSetting(configuredSyncsMap[configuredSyncs.at(num)]);
    configuredSyncsMap.remove(configuredSyncs.at(num));
    configuredSyncs.removeAt(num);
    mutex.unlock();
}

void Model::removeSyncedFolderByTag(int tag)
{
    mutex.lock();
    assert(preferences->logged());

    preferences->removeSyncSetting(configuredSyncsMap[tag]);
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

    mutex.unlock();
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
    }
    configuredSyncs.clear();
    configuredSyncsMap.clear();
    loadedSyncsMap.clear();
}

void Model::updateSyncSettings(MegaSync *sync, const char *remotePath)
{
    if (!sync)
    {
        return;
    }

    QMutexLocker qm(&mutex);
    assert(preferences->logged());

    std::shared_ptr<SyncSetting> cs;

    if (configuredSyncsMap.contains(sync->getTag()))
    {
        cs = configuredSyncsMap[sync->getTag()];
        cs->setSync(sync);

        assert(remotePath || cs->getMegaFolder().size()); //updated syncs should always have a remote path
        if (remotePath)
        {
            cs->setMegaFolder(remotePath);
        }
    }
    else
    {
        cs = configuredSyncsMap[sync->getTag()] = std::make_shared<SyncSetting>(sync, remotePath);
        configuredSyncs.append(sync->getTag());
    }

    preferences->writeSyncSetting(cs);

    emit syncStateChanged(cs);
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
    return QString::number(configuredSyncsMap[configuredSyncs.at(num)]->tag());
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

QDataStream& operator<<(QDataStream& out, const SyncSetting& v)
{
    out << v.tag() << v.name();
    return out;
}

QDataStream& operator>>(QDataStream& in, SyncSetting& v) {
    decltype(v.tag()) tag; in >> tag; v.setTag(tag);
    decltype(v.name()) name; in >> name; v.setName(name);
    return in;
}
