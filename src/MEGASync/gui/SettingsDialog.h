#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "AccountDetailsDialog.h"
#include "DownloadFromMegaDialog.h"
#include "ChangePassword.h"
#include "Preferences/Preferences.h"
#include "control/Utilities.h"
#include "syncs/model/SyncItemModel.h"
#include "syncs/model/BackupItemModel.h"
#include "syncs/gui/Backups/BackupTableViewTooltips.h"
#include "syncs/gui/Twoways/SyncTableViewTooltips.h"

#include "syncs/control/SyncController.h"
#include "syncs/control/SyncInfo.h"
#include "megaapi.h"

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>

#ifdef Q_OS_MACOS
#include "platform/macx/QCustomMacToolbar.h"
#endif

namespace Ui {
class SettingsDialog;
}

class ProxySettings;
class MegaApplication;

class SettingsDialog : public QDialog, public IStorageObserver, public IBandwidthObserver,
        public IAccountObserver
{
    Q_OBJECT

public:
    enum Tabs{
        GENERAL_TAB  = 0,
        ACCOUNT_TAB  = 1,
        SYNCS_TAB    = 2,
        BACKUP_TAB    = 3,
        SECURITY_TAB = 4,
        FOLDERS_TAB  = 5,
        NETWORK_TAB  = 6,
        NOTIFICATIONS_TAB = 7
    };
    Q_ENUM(Tabs)

    explicit SettingsDialog(MegaApplication* app, bool proxyOnly = false, QWidget* parent = nullptr);
    ~SettingsDialog();
    void openSettingsTab(int tab = -1);
    void setProxyOnly(bool proxyOnly);

    // General
    void setOverQuotaMode(bool mode);
    void setUpdateAvailable(bool updateAvailable);
    void storageChanged();

    // Account
    void updateStorageElements() override;
    void updateBandwidthElements() override;
    void updateAccountElements() override;

    // Syncs
    enum SyncStateInformation
    {
        SAVING_SYNCS = 0,
        SAVING_BACKUPS,
        SAVING_SYNCS_FINISHED,
        SAVING_BACKUPS_FINISHED,
    };
    void addSyncFolder(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE);

    // Folders
    void updateUploadFolder();
    void updateDownloadFolder();

signals:
    void userActivity();

public slots:
    // Network
    void showGuestMode();

    // General
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();

    // Account
    void storageStateChanged(int state);

private slots:
    void on_bHelp_clicked();
#ifdef Q_OS_MACOS
    void onAnimationFinished();
    void initializeNativeUIComponents();
#endif

    // General
    void on_bGeneral_clicked();
    void on_bClearCache_clicked();
    void on_bClearRemoteCache_clicked();
    void on_bClearFileVersions_clicked();
    void on_cCacheSchedulerEnabled_toggled();
    void on_sCacheSchedulerDays_valueChanged(int i);
    void on_cAutoUpdate_toggled(bool checked);
    void on_cStartOnStartup_toggled(bool checked);
    void on_cLanguage_currentIndexChanged(int index);
    void on_cFileVersioning_toggled(bool checked);
    void on_cbSleepMode_toggled(bool checked);
    void on_cOverlayIcons_toggled(bool checked);
#ifdef Q_OS_WINDOWS
    void on_cFinderIcons_toggled(bool checked);
#endif
    void on_bUpdate_clicked();
    void on_bFullCheck_clicked();
    void on_bSendBug_clicked();

    // Account
    void on_bAccount_clicked();
    void on_lAccountType_clicked();
    void on_bUpgrade_clicked();
    void on_bBuyMoreSpace_clicked();
    void on_bMyAccount_clicked();
    void on_bStorageDetails_clicked();
    void on_bLogout_clicked();

    // Syncs

    void on_bSyncs_clicked();
    void on_bAddSync_clicked();
    void on_bDeleteSync_clicked();
#ifndef WIN32
    void on_bPermissions_clicked();
#endif

    void onSavingSyncsCompleted(SyncStateInformation value);

    // Backup
    void on_bBackup_clicked();
    void on_bAddBackup_clicked();
    void on_bDeleteBackup_clicked();
    void removeBackup(std::shared_ptr<SyncSettings> backup);
    void removeSync(std::shared_ptr<SyncSettings> sync);
    void on_bOpenBackupFolder_clicked();
    void openHandleInMega(mega::MegaHandle handle);
    void on_bBackupCenter_clicked();
    void onMyBackupsFolderHandleSet(mega::MegaHandle h);

    // Security
    void on_bSecurity_clicked();
    void on_bExportMasterKey_clicked();
    void on_bChangePassword_clicked();
    void on_bSessionHistory_clicked();

    // Folders
    void on_bFolders_clicked();
    void on_bUploadFolder_clicked();
    void on_bDownloadFolder_clicked();
    void on_bAddName_clicked();
    void on_bDeleteName_clicked();
    void on_cExcludeUpperThan_clicked();
    void on_cExcludeLowerThan_clicked();
    void on_eUpperThan_valueChanged(int i);
    void on_eLowerThan_valueChanged(int i);
    void on_cbExcludeUpperUnit_currentIndexChanged(int index);
    void on_cbExcludeLowerUnit_currentIndexChanged(int index);
    void on_bRestart_clicked();

    // Network
    void on_bNetwork_clicked();
    void on_bOpenProxySettings_clicked();
    void on_bOpenBandwidthSettings_clicked();

    //Notifications
    void on_bNotifications_clicked();

protected:
    void changeEvent(QEvent* event) override;
#ifdef Q_OS_MACOS
    void closeEvent(QCloseEvent * event);
#endif

private slots:
    void onShellNotificationsProcessed();
#ifdef Q_OS_MACOS
    // Due to issues with QT and window manager on macOS, menus are not closing when
    // you close settings dialog using close toolbar button. To fix it, emit a signal when about to close
    // and force to close the sync menu (if visible)
    void closeMenus();
#endif

private:
    void connectSyncHandlers();
    void loadSyncSettings();
    void connectBackupHandlers();
    void loadBackupSettings();

    void loadSettings();
    void onCacheSizeAvailable();
    void saveExcludeSyncNames();
    void updateNetworkTab();
    void setShortCutsForToolBarItems();
    void updateCacheSchedulerDaysLabel();

    void syncsStateInformation(SyncStateInformation state);

    void addSyncFolderAfterOverQuotaCheck(mega::MegaHandle megaFolderHandle);
    void connectAddSyncHandler();

    void setEnabledAllControls(const bool enabled);
    void setGeneralTabEnabled(const bool enabled);

#ifdef Q_OS_MACOS
    void reloadToolBarItemNames();
    void macOSretainSizeWhenHidden();
    void animateSettingPage(int endValue, int duration = 150);
    QPropertyAnimation* mMinHeightAnimation;
    QPropertyAnimation* mMaxHeightAnimation;
    QParallelAnimationGroup* mAnimationGroup;
    QCustomMacToolbar* mToolBar;
    QMacToolBarItem *bGeneral;
    QMacToolBarItem *bAccount;
    QMacToolBarItem *bSyncs;
    QMacToolBarItem *bBackup;
    QMacToolBarItem *bSecurity;
    QMacToolBarItem *bFolders;
    QMacToolBarItem* bNetwork;
    QMacToolBarItem* bNotifications;
#endif

    Ui::SettingsDialog* mUi;
    MegaApplication* mApp;
    std::shared_ptr<Preferences> mPreferences;
    SyncController mSyncController;
    SyncController mBackupController;
    SyncInfo* mModel;
    mega::MegaApi* mMegaApi;
    bool mProxyOnly;
    int mLoadingSettings;
    ThreadPool* mThreadPool;
    QStringList mLanguageCodes;
    QFutureWatcher<long long> mCacheSizeWatcher;
    QFutureWatcher<long long> mRemoteCacheSizeWatcher;
    long long mCacheSize;
    long long mRemoteCacheSize;
    int mDebugCounter; // Easter Egg
    QStringList mSyncNames;
    bool mHasDefaultUploadOption;
    bool mHasDefaultDownloadOption;
    std::unique_ptr<SyncTableViewTooltips> mSyncTableEventFilter;
    std::unique_ptr<BackupTableViewTooltips> mBackupTableEventFilter;
};
#endif // SETTINGSDIALOG_H
