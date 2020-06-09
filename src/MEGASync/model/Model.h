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
 * The idea of this class is to hold any state variable data.
 * and proceed to to persist that data when required.
 * It indens to alliviate Preferences from that burden.
 *
 * The Model would cover the funcionality related with with
 * updating of the data it manages (e.g: when a SyncConfiguration is updated,
 * it will call platform specific callbacks that deal with that)
 *
 *
 * The model will be updated when required and trigger events
 * when changes occur that should be noted by UI classes.
 *
 */
class Model : public QObject
{
    Q_OBJECT

signals:
    void syncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void syncRemoved(std::shared_ptr<SyncSetting> syncSettings);

private:
    static Model *model;
    Model();

    ///////////////// SYNCS ///////////////////////
    std::map<QString, QVariant> cache;
    Preferences *preferences;
    bool isFirstSyncDone = false;
    ///////////// END OF SYNCS ////////////////////

protected:
    QMutex syncMutex;

    ///////////////// SYNCS ///////////////////////
    void removeSyncSetting(std::shared_ptr<SyncSetting> syncSettings);
    void writeSyncSetting(std::shared_ptr<SyncSetting> syncSettings);

    QList<int> configuredSyncs; //Tags of configured syncs
    QMap<int, std::shared_ptr<SyncSetting>> configuredSyncsMap;


    ///////////// END OF SYNCS ////////////////////

    void onSyncStateChanged(std::shared_ptr<mega::MegaSync> sync);

public:
    void reset();
    static Model *instance();

    ///////////////// SYNCS ///////////////////////
    // TODO: doc all these

    std::shared_ptr<SyncSetting> updateSyncSettings(mega::MegaSync *sync, int addingState = 0);
    void activateSync(std::shared_ptr<SyncSetting> cs);
    void deactivateSync(std::shared_ptr<SyncSetting> cs);

    void pickInfoFromOldSync(const SyncData &osd, int tag);
    void removeSyncedFolder(int num);
    void removeSyncedFolderByTag(int tag);
    void removeAllFolders();

    void removeOldCachedSync(int position);
    QList<SyncData> readOldCachedSyncs();//get a list of cached syncs (withouth loading them in memory): intended for transition to sdk caching them.
    void saveOldCachedSyncs(); //save the old cache (intended to clean them)

    std::shared_ptr<SyncSetting> getSyncSetting(int num);
    std::shared_ptr<SyncSetting> getSyncSettingByTag(int num);

    int getNumSyncedFolders();

    QStringList getSyncNames();
    QStringList getSyncIDs();
    QStringList getMegaFolders();
    QStringList getLocalFolders();
    QList<long long> getMegaFolderHandles();

    ///////////// END OF SYNCS ////////////////////

};
