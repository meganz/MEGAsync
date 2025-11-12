#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "AppState.h"
#include "megaapi.h"
#include "Preferences.h"
#include "SyncInfo.h"
#include "UsersUpdateListener.h"
#include "Utilities.h"

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>

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
        ACCOUNT_TAB = 1,
        SYNCS_TAB = 2,
        BACKUP_TAB = 3,
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
    void setUpdateAvailable(bool updateAvailable);
    void storageChanged();

    // Account
    void updateStorageElements() override;
    void updateBandwidthElements() override;
    void updateAccountElements() override;

    // Syncs
    void on_bSyncs_clicked();

    // Backup
    void on_bBackup_clicked();

    // Folders
    void updateUploadFolder();
    void updateDownloadFolder();

    void setSyncAddButtonEnabled(bool enabled,
                                 SettingsDialog::Tabs tab = SettingsDialog::Tabs::SYNCS_TAB);
    void setChangePasswordEnabled(bool enabled);

signals:
    void userActivity();

public slots:
    // React to AppState change
    void onAppStateChanged(AppState::AppStates oldAppState, AppState::AppStates newAppState);

    // Network
    void showGuestMode();

    // General
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();

    //Enable/Disable controls
    void setEnabledAllControls(const bool enabled);

private slots:
    void on_bBackupCenter_clicked();
    void on_bHelp_clicked();

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
    void on_cDesktopIntegration_toggled(bool checked);
#endif
    void on_cbTheme_currentIndexChanged(int index);
    void on_bUpdate_clicked();
    void on_bSendBug_clicked();

    // Account
    void on_bAccount_clicked();
    void on_lAccountType_clicked();
    void on_bUpgrade_clicked();
    void on_bMyAccount_clicked();
    void on_bStorageDetails_clicked();
    void on_bLogout_clicked();

    // Security
    void on_bSecurity_clicked();
    void on_bExportMasterKey_clicked();
    void on_bChangePassword_clicked();
    void on_bSessionHistory_clicked();

    // Folders
    void on_bFolders_clicked();
    void on_bUploadFolder_clicked();
    void on_bDownloadFolder_clicked();

    // Network
    void on_bNetwork_clicked();
    void on_bOpenProxySettings_clicked();
    void on_bOpenBandwidthSettings_clicked();

    //Notifications
    void on_bNotifications_clicked();

protected:
    bool event(QEvent* event) override;

private slots:
    void onShellNotificationsProcessed();
    void onUserEmailChanged(mega::MegaHandle userHandle, const QString& newEmail);
    void onRequestTaskbarPinningTimeout();

private:
    void loadSettings();
    void onCacheSizeAvailable();
    void saveExcludeSyncNames();
    void updateNetworkTab();
    void setShortCutsForToolBarItems();
    void updateCacheSchedulerDaysLabel();
    void setGeneralTabEnabled(const bool enabled);
    void setOverlayCheckboxEnabled(const bool enabled, const bool checked);
    void setProgressState(const QString& stateName);
    void startRequestTaskbarPinningTimer();
    void initColorTheme();

    Ui::SettingsDialog* mUi;
    MegaApplication* mApp;
    std::shared_ptr<Preferences> mPreferences;
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
    std::unique_ptr<UsersUpdateListener> usersUpdateListener;
    QTimer* mTaskbarPinningRequestTimer;
};
#endif // SETTINGSDIALOG_H
