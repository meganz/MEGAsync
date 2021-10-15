#pragma once

#include <memory>

#include <QString>
#include <QStringList>
#include <QSet>
#include <QMutex>

#include "control/Preferences.h"
#include "SyncSettings.h"

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

signals:
    void syncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void syncRemoved(std::shared_ptr<SyncSetting> syncSettings);
    void syncDisabledListUpdated();

private:
    static std::unique_ptr<SyncModel> model;
    SyncModel();

    Preferences *preferences;
    bool isFirstSyncDone = false;
    QString mDeviceName;
    mega::MegaHandle mBackupsDirHandle;

    void saveUnattendedDisabledSyncs();

protected:
    QMutex syncMutex;

    QMap<mega::MegaSync::SyncType, QList<mega::MegaHandle>> configuredSyncs; //Tags of configured syncs
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> configuredSyncsMap;
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> syncsSettingPickedFromOldConfig;

    QSet<mega::MegaHandle> unattendedDisabledSyncs; //Tags of syncs disabled due to errors since last dismissed

public:
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
    void removeSyncedFolder(int num, mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void removeSyncedFolderByBackupId(mega::MegaHandle backupId);
    void removeAllFolders();

    // Getters
    std::shared_ptr<SyncSetting> getSyncSetting(int num, mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    std::shared_ptr<SyncSetting> getSyncSettingByTag(mega::MegaHandle tag);
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> getCopyOfSettings();

    int getNumSyncedFolders(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);

    // FIXME: Remove unattended disabled syncs
    bool hasUnattendedDisabledSyncs() const;
    void addUnattendedDisabledSync(mega::MegaHandle tag);
    void removeUnattendedDisabledSync(mega::MegaHandle tag);
    void setUnattendedDisabledSyncs(QSet<mega::MegaHandle> tags);
    void dismissUnattendedDisabledSyncs();
    // --

    QStringList getSyncNames(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    QStringList getSyncIDs(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    QStringList getMegaFolders(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    QStringList getLocalFolders(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    QList<mega::MegaHandle> getMegaFolderHandles(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);

    bool isRemoteRootSynced();

    void updateMegaFolder(QString newRemotePath, std::shared_ptr<SyncSetting> cs);
};
