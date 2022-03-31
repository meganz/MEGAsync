#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "AccountDetailsDialog.h"
#include "BindFolderDialog.h"
#include "DownloadFromMegaDialog.h"
#include "ChangePassword.h"
#include "Preferences.h"
#include "MegaController.h"
#include "../model/Model.h"
#include "megaapi.h"
#include "HighDpiResize.h"
#include "control/Utilities.h"

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
    enum {
        GENERAL_TAB  = 0,
        ACCOUNT_TAB  = 1,
        SYNCS_TAB    = 2,
        SECURITY_TAB = 3,
        FOLDERS_TAB  = 4,
        NETWORK_TAB  = 5,
        NOTIFICATIONS_TAB = 6
        };

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
    enum SyncStateInformation {NO_SAVING_SYNCS = 0, SAVING_SYNCS = 1};
    void loadSyncSettings();
    void addSyncFolder(mega::MegaHandle megaFolderHandle);

    // Folders
    void updateUploadFolder();
    void updateDownloadFolder();

signals:
    void userActivity();
#ifdef Q_OS_MACOS
    // Due to issues with QT and window manager on macOS, menus are not closing when
    // you close settings dialog using close toolbar button. To fix it, emit a signal when about to close
    // and force to close the sync menu (if visible)
    void closeMenus();
#endif

public slots:
    // Network
    void showGuestMode();

    // General
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();

    // Account
    void storageStateChanged(int state);

    // Syncs
    void syncStateChanged(int state);
    void onSyncStateChanged(std::shared_ptr<SyncSetting>);
    void onEnableSyncFailed(int, std::shared_ptr<SyncSetting> syncSetting);
    void onDisableSyncFailed(std::shared_ptr<SyncSetting> syncSetting);
    void onSyncSelected(const QItemSelection& selected, const QItemSelection& deselected);

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
    void setAvatar();

    // Syncs
    void onSavingSyncsProgress(double progress);
    void onSavingSyncsCompleted();
    void on_bSyncs_clicked();
    void on_bAdd_clicked();
    void on_bDelete_clicked();
    void on_tSyncs_doubleClicked(const QModelIndex &index);
    void onCellClicked(int row, int column);
    void showInFolderClicked();
    void showInMegaClicked();
    void onDeleteSync();
#ifndef WIN32
    void on_bPermissions_clicked();
#endif

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

    void restartApp();

private:
    void loadSettings();
    void onCacheSizeAvailable();
    void saveSyncSettings();
    void savingSyncs(bool completed, QObject* item);
    void syncsStateInformation(int state);
    void addSyncRow(int row, const QString& name, const QString& lPath,
                    const QString& rPath, bool isActive, int error, mega::MegaHandle megaHandle,
                    mega::MegaHandle tag, std::shared_ptr<SyncSetting> syncSetting = nullptr);
    void saveExcludeSyncNames();
    void updateNetworkTab();
    void setShortCutsForToolBarItems();

    enum
    {
        SYNC_COL_ENABLE_CB = 0,
        SYNC_COL_LFOLDER   = 1,
        SYNC_COL_RFOLDER   = 2,
        SYNC_COL_MENU      = 3,
        SYNC_COL_TAG       = 4,
        SYNC_COL_HANDLE    = 5,
        SYNC_COL_NAME      = 6,
        SYNC_COL_NB
    };

#ifdef Q_OS_MACOS
    void reloadToolBarItemNames();
    void macOSretainSizeWhenHidden();
    void animateSettingPage(int endValue, int duration = 150);
    QPropertyAnimation* mMinHeightAnimation;
    QPropertyAnimation* mMaxHeightAnimation;
    QParallelAnimationGroup* mAnimationGroup;
    std::unique_ptr<QCustomMacToolbar> mToolBar;
    std::unique_ptr<QMacToolBarItem> bGeneral;
    std::unique_ptr<QMacToolBarItem> bAccount;
    std::unique_ptr<QMacToolBarItem> bSyncs;
    std::unique_ptr<QMacToolBarItem> bSecurity;
    std::unique_ptr<QMacToolBarItem> bFolders;
    std::unique_ptr<QMacToolBarItem> bNetwork;
    std::unique_ptr<QMacToolBarItem> bNotifications;
#endif

    Ui::SettingsDialog* mUi;
    MegaApplication* mApp;
    Preferences* mPreferences;
    Controller* mController;
    Model* mModel;
    mega::MegaApi* mMegaApi;
    HighDpiResize mHighDpiResize;
    bool mProxyOnly;
    int mLoadingSettings;
    ThreadPool* mThreadPool;
    QStringList mLanguageCodes;
    QFutureWatcher<long long> mCacheSizeWatcher;
    QFutureWatcher<long long> mRemoteCacheSizeWatcher;
    AccountDetailsDialog* mAccountDetailsDialog;
    long long mCacheSize;
    long long mRemoteCacheSize;
    int mDebugCounter; // Easter Egg
    QStringList mSyncNames;
    bool mAreSyncsDisabled; //Check if there are any sync disabled by any kind of error
    bool mIsSavingSyncsOnGoing;
    int mSelectedSyncRow;
    std::unique_ptr<ProgressHelper> mSaveSyncsProgress;
    bool mHasDefaultUploadOption;
    bool mHasDefaultDownloadOption;
    QPointer<ProxySettings> mProxySettingsDialog;
};
#endif // SETTINGSDIALOG_H
