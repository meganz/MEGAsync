#pragma once

#include <memory>

#include <QString>
#include <QStringList>
#include <QSet>
#include <QMutex>
#include <QVector>

#include "control/Preferences.h"
#include "model/SyncSettings.h"

#include "megaapi.h"

class Preferences;

/**
 * @brief The Sync Model class
 *
 * The idea of this class is to hold any state variable data and proceed to persist that
 * data when required. It is intended to alleviate Preferences from that burden.
 *
 * The SyncModel covers the functionality related with updating the data it manages
 * (e.g: when a SyncConfiguration is updated, it will call platform specific callbacks that
 *  deal with that)
 *
 * The model will be updated when required and trigger events when changes occur that should
 * be noted by UI classes.
 *
 */

class SyncModel : public QObject
{
    Q_OBJECT
    using SyncType = mega::MegaSync::SyncType;

signals:
    void syncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void syncRemoved(std::shared_ptr<SyncSetting> syncSettings);
    void syncDisabledListUpdated();

private:
    static std::unique_ptr<SyncModel> model;
    SyncModel();

    std::shared_ptr<Preferences> preferences;
    bool isFirstSyncDone = false;

    void saveUnattendedDisabledSyncs();

protected:
    QMutex syncMutex;

    QMap<SyncType, QList<mega::MegaHandle>> configuredSyncs; //Tags of configured syncs
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> configuredSyncsMap;
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> syncsSettingPickedFromOldConfig;
    QMap<SyncType, QSet<mega::MegaHandle>> unattendedDisabledSyncs; //Tags of syncs disabled due to errors since last dismissed

public:
    static const QVector<SyncType> AllHandledSyncTypes;

    void reset();
    static SyncModel *instance();

    /**
     * @brief Updates sync model
     * @param sync MegaSync object with the required information
     * @param addingState to distinguish adding cases (from onSyncAdded)
     * @return
     */
    std::shared_ptr<SyncSetting> updateSyncSettings(mega::MegaSync *sync, int addingState = 0);

    // transition sync to active: will trigger platform dependent behaviour
    void activateSync(std::shared_ptr<SyncSetting> cs);
    // transition sync to inactive: will trigger platform dependent behaviour
    void deactivateSync(std::shared_ptr<SyncSetting> cs);

    // store all megasync specific info of syncsettings into megasync cache
    void rewriteSyncSettings();

    // load into sync model the information from an old cached sync
    void pickInfoFromOldSync(const SyncData &osd, mega::MegaHandle backupId, bool loadedFromPreviousSessions);

    // remove syncs from model
    void removeSyncedFolderByBackupId(mega::MegaHandle backupId);
    void removeAllFolders();

    // Getters
    std::shared_ptr<SyncSetting> getSyncSetting(int num, SyncType type);
    std::shared_ptr<SyncSetting> getSyncSettingByTag(mega::MegaHandle tag);
    QList<std::shared_ptr<SyncSetting>> getSyncSettingsByType(const QVector<SyncType>& types);
    QList<std::shared_ptr<SyncSetting>> getSyncSettingsByType(SyncType type)
        {return getSyncSettingsByType(QVector<SyncType>({type}));}
    QList<std::shared_ptr<SyncSetting>> getAllSyncSettings()
        {return getSyncSettingsByType(AllHandledSyncTypes);}

    int getNumSyncedFolders(const QVector<mega::MegaSync::SyncType>& types);
    int getNumSyncedFolders(SyncType type)
        {return getNumSyncedFolders(QVector<SyncType>({type}));}

    bool syncWithErrorExist(const QVector<SyncType>& types);
    bool syncWithErrorExist(SyncType type)
        {return syncWithErrorExist(QVector<SyncType>({type}));}
    bool hasUnattendedDisabledSyncs(const QVector<SyncType>& types) const;
    bool hasUnattendedDisabledSyncs(SyncType type) const
        {return hasUnattendedDisabledSyncs(QVector<SyncType>({type}));}
    void addUnattendedDisabledSync(mega::MegaHandle tag, SyncType type);
    void removeUnattendedDisabledSync(mega::MegaHandle tag, SyncType type);
    void setUnattendedDisabledSyncs(const QSet<mega::MegaHandle>& tags);
    void dismissUnattendedDisabledSyncs(const QVector<SyncType>& types);
    void dismissUnattendedDisabledSyncs(SyncType type)
        {return dismissUnattendedDisabledSyncs(QVector<SyncType>({type}));}

    QStringList getSyncNames(const QVector<SyncType>& types);
    QStringList getSyncNames(SyncType type)
        {return getSyncNames(QVector<SyncType>({type}));}
    QStringList getSyncIDs(const QVector<SyncType>& types);
    QStringList getSyncIDs(SyncType type)
        {return getSyncIDs(QVector<SyncType>({type}));}
    QStringList getMegaFolders(const QVector<SyncType>& types);
    QStringList getMegaFolders(SyncType type)
        {return getMegaFolders(QVector<SyncType>({type}));}
    QStringList getLocalFolders(const QVector<SyncType>& types);
    QStringList getLocalFolders(SyncType type)
        {return getLocalFolders(QVector<SyncType>({type}));}
    QMap<QString, SyncType> getLocalFoldersAndTypeMap();
    QList<mega::MegaHandle> getMegaFolderHandles(const QVector<SyncType>& types);
    QList<mega::MegaHandle> getMegaFolderHandles(SyncType type)
        {return getMegaFolderHandles(QVector<SyncType>({type}));}
    //cloudDrive = true: only cloud drive mega folders. If false will return only inshare syncs.
    QStringList getCloudDriveSyncMegaFolders(bool cloudDrive = true);

    void updateMegaFolder(QString newRemotePath, std::shared_ptr<SyncSetting> cs);
};
