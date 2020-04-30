#pragma once

#include <iostream>
#include <QString>
#include <QLocale>
#include <QStringList>
#include <QMutex>
#include <QDataStream>

#include <assert.h>
#include <memory>

//TODO: reduce includes

#include "control/Preferences.h"
#include "SyncSettings.h"

#include "megaapi.h"

class Preferences;
/**
 * @brief The Model class
 *
 * The idea is to encapsulate here all the use-cases in the app
 * and hold any state variable data.
 */
class Model : public QObject
{
    Q_OBJECT

signals:
    void stateChanged();
    void updated(int lastVersion);
    void onSyncStateChanged(int tag);

private:
    static Model *model;
    Model();

    ///////////////// SYNCS ///////////////////////
    std::map<QString, QVariant> cache;
    Preferences *preferences;
    ///////////// END OF SYNCS ////////////////////

protected:
    QMutex mutex; //TODO: rename to syncMutex

    ///////////////// SYNCS ///////////////////////
    void removeSyncSetting(std::shared_ptr<SyncSetting> syncSettings);
    void writeSyncSetting(std::shared_ptr<SyncSetting> syncSettings);

    QList<int> configuredSyncs; //Tags of configured syncs
    QMap<int, std::shared_ptr<SyncSetting>> configuredSyncsMap;

    // loaded syncs at startup //TODO: extend docs on all these
    QMap<int, std::shared_ptr<SyncSetting>> loadedSyncsMap;
    ///////////// END OF SYNCS ////////////////////

    void onSyncStateChanged(std::shared_ptr<mega::MegaSync> sync);

public:
    void reset();
    static Model *instance();

    ///////////////// SYNCS ///////////////////////
    // TODO: doc all these
    void updateSyncSettings(mega::MegaSync *sync, const char *remotePath = nullptr);
    void pickInfoFromOldSync(const SyncData &osd, int tag);
    void removeSyncedFolder(int num);
    void removeSyncedFolderByTag(int tag);
    void removeAllFolders();

    void removeOldCachedSync(int position);
    QList<SyncData> readOldCachedSyncs();//get a list of cached syncs (withouth loading them in memory): intended for transition to sdk caching them.
    void saveOldCachedSyncs(); //save the old cache (intended to clean them)

    std::shared_ptr<SyncSetting> getSyncSetting(int num);

    int getNumSyncedFolders();

    //TODO: try to get rid of the following and use SyncSetting objects
    QString getSyncName(int num);
    QString getSyncID(int num);
    QString getLocalFolder(int num);
    QString getMegaFolder(int num);
    long long getLocalFingerprint(int num);
    mega::MegaHandle getMegaFolderHandle(int num);
    bool isFolderActive(int num);
    bool isTemporaryInactiveFolder(int num);

    QStringList getSyncNames();
    QStringList getSyncIDs();
    QStringList getMegaFolders();
    QStringList getLocalFolders();
    QList<long long> getMegaFolderHandles();

    // when login/entering some user settins.

//    QStringList getExcludedSyncNames();
//    void setExcludedSyncNames(QStringList names);
//    QStringList getExcludedSyncPaths();
//    void setExcludedSyncPaths(QStringList paths);

    ///////////// END OF SYNCS ////////////////////

};
