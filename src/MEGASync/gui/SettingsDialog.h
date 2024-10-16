#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "Preferences.h"
#include "Utilities.h"

#include "SyncInfo.h"
#include "megaapi.h"

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>

namespace Ui {
class SettingsDialog;
class SyncStallModeSelector;
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
    void onSmartModeSelected(bool checked);
    void onAdvanceModeSelected(bool checked);
    void onPreferencesValueChanged(QString key);
#ifndef Q_OS_WINDOWS
    void onPermissionsClicked();
#endif
    void applyPreviousExclusions();
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
    void on_cbTheme_currentIndexChanged(int index);
    void on_bUpdate_clicked();
    void on_bFullCheck_clicked();
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
    void changeEvent(QEvent* event) override;

private slots:
    void onShellNotificationsProcessed();

private:
    void loadSettings();
    void onCacheSizeAvailable();
    void saveExcludeSyncNames();
    void updateNetworkTab();
    void setShortCutsForToolBarItems();
    void showUnexpectedSyncError(const QString& message);
    void updateCacheSchedulerDaysLabel();
    void setGeneralTabEnabled(const bool enabled);
    void setOverlayCheckboxEnabled(const bool enabled, const bool checked);
    void setProgressState(const QString& stateName, int value);

    Ui::SettingsDialog* mUi;
    Ui::SyncStallModeSelector* syncStallModeSelectorUI;
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
};
#endif // SETTINGSDIALOG_H
