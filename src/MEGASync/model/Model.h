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
 * @brief The Model class
 *
 * The idea of this class is to hold any state variable data.
 * and proceed to persist that data when required.
 * It intends to alliviate Preferences from that burden.
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
    void syncDisabledListUpdated();

private:
    static std::unique_ptr<Model> model;
    Model();

    ///////////////// SYNCS ///////////////////////
    std::shared_ptr<Preferences> preferences;
    bool isFirstSyncDone = false;
    ///////////// END OF SYNCS ////////////////////

    void saveUnattendedDisabledSyncs();

protected:
    QMutex syncMutex;

    ///////////////// SYNCS ///////////////////////
    QList<mega::MegaHandle> configuredSyncs; //Tags of configured syncs
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> configuredSyncsMap;
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> syncsSettingPickedFromOldConfig;

    QSet<mega::MegaHandle> unattendedDisabledSyncs; //Tags of syncs disabled due to errors since last dismissed
    ///////////// END OF SYNCS ////////////////////


public:
    void reset();
    static Model *instance();

    ///////////////// SYNCS ///////////////////////

    /**
     * @brief Updates sync model
     * @param sync MegaSync object with the required information
     * @param addingState to distinguish adding cases (from onSyncAdded)
     * @return
     */
    std::shared_ptr<SyncSetting> updateSyncSettings(mega::MegaSync *sync);

    // transition sync to active: will trigger platform dependent behaviour
    void activateSync(std::shared_ptr<SyncSetting> cs);
    // transition sync to inactive: will trigger platform dependent behaviour
    void deactivateSync(std::shared_ptr<SyncSetting> cs);

    // store all megasync specific info of syncsettings into megasync cache
    void rewriteSyncSettings();

    // load into sync model the information from an old cached sync
    void pickInfoFromOldSync(const SyncData &osd, mega::MegaHandle backupId, bool loadedFromPreviousSessions);

    // remove syncs from model
    void removeSyncedFolder(int num);
    void removeSyncedFolderByBackupId(mega::MegaHandle backupId);
    void removeAllFolders();

    // Getters
    std::shared_ptr<SyncSetting> getSyncSetting(int num);
    std::shared_ptr<SyncSetting> getSyncSettingByTag(mega::MegaHandle num);
    QMap<mega::MegaHandle, std::shared_ptr<SyncSetting>> getCopyOfSettings();

    int getNumSyncedFolders();

    //unattended disabled syncs
    bool hasUnattendedDisabledSyncs() const;
    void addUnattendedDisabledSync(mega::MegaHandle tag);
    void removeUnattendedDisabledSync(mega::MegaHandle tag);
    void setUnattendedDisabledSyncs(QSet<mega::MegaHandle> tags);
    void dismissUnattendedDisabledSyncs();

    QStringList getSyncNames();
    QStringList getSyncIDs();
    QStringList getMegaFolders();

    //cloudDrive = true: only cloud drive mega folders. If false will return only inshare syncs.
    QStringList getCloudDriveSyncMegaFolders(bool cloudDrive = true);
    QString getMegaFolderByHandle(const mega::MegaHandle& handle);
    QStringList getLocalFolders();
    QList<mega::MegaHandle> getMegaFolderHandles();

    ///////////// END OF SYNCS ////////////////////

    void updateMegaFolder(QString newRemotePath, std::shared_ptr<SyncSetting> cs);
};
