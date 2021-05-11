#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>

#ifdef Q_OS_MACOS
#include <QMacToolBar>
#endif

#include "AccountDetailsDialog.h"
#include "BindFolderDialog.h"
#include "SizeLimitDialog.h"
#include "LocalCleanScheduler.h"
#include "DownloadFromMegaDialog.h"
#include "ChangePassword.h"
#include "Preferences.h"
#include "MegaController.h"
#include "../model/Model.h"
#include "megaapi.h"
#include "HighDpiResize.h"
#include "control/Utilities.h"

namespace Ui {
class SettingsDialog;
}

class ProxySettings;

class MegaApplication;
class SettingsDialog : public QDialog, public IStorageObserver, public IBandwidthObserver, public IAccountObserver
{
    Q_OBJECT

public:
    enum {GENERAL_TAB = 0, ACCOUNT_TAB = 1, SYNCS_TAB = 2, IMPORTS_TAB = 3, NETWORK_TAB = 4, SECURITY_TAB = 5};
    enum SyncStateInformation {NO_SAVING_SYNCS = 0, SAVING_SYNCS = 1};

    explicit SettingsDialog(MegaApplication *app, bool proxyOnly = false, QWidget *parent = 0);
    ~SettingsDialog();

    void setProxyOnly(bool proxyOnly);
    void setOverQuotaMode(bool mode);
    void refreshAccountDetails();
    void setUpdateAvailable(bool updateAvailable);
    void openSettingsTab(int tab = -1);
    void storageChanged();
    void addSyncFolder(mega::MegaHandle megaFolderHandle);
    void loadSyncSettings();
    void updateUploadFolder();
    void updateDownloadFolder();

    void updateStorageElements() override;
    void updateBandwidthElements() override;
    void updateAccountElements() override;

signals:
    void userActivity();

public slots:
    void storageStateChanged(int state);
    void syncStateChanged(int state);
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();
    void onSyncStateChanged(std::shared_ptr<SyncSetting>);
    void onEnableSyncFailed(int, std::shared_ptr<SyncSetting> syncSetting);
    void onDisableSyncFailed(std::shared_ptr<SyncSetting> syncSetting);
    void showGuestMode();

private slots:
    void onSavingSettingsProgress(double progress);
    void onSavingSettingsCompleted();

    void on_bGeneral_clicked();
    void on_bAccount_clicked();
    void on_bSyncs_clicked();
    void on_bNetwork_clicked();
    void on_bSecurity_clicked();
    void on_bImports_clicked();

    void on_bHelp_clicked();
#ifndef __APPLE__
    void on_bHelpIco_clicked();
#endif

    void on_bUpgrade_clicked();
    void on_bUpgradeBandwidth_clicked();


    void on_bAdd_clicked();
    void on_bDelete_clicked();
    void on_bExcludeSize_clicked();
    void on_bLocalCleaner_clicked();


    void on_tSyncs_doubleClicked(const QModelIndex &index);
    void on_bUploadFolder_clicked();
    void on_bDownloadFolder_clicked();

    void on_bAddName_clicked();
    void on_bDeleteName_clicked();
    void on_bClearCache_clicked();
    void on_bClearRemoteCache_clicked();
    void on_bClearFileVersions_clicked();
    void on_bUpdate_clicked();
    void on_bStorageDetails_clicked();
    void on_lAccountImage_clicked();
    void on_bSendBug_clicked();

    void setAvatar();

    void on_cShowNotifications_toggled(bool checked);
    void on_cAutoUpdate_toggled(bool checked);
    void on_cStartOnStartup_toggled(bool checked);
    void on_cLanguage_currentIndexChanged(int index);
    void on_eUploadFolder_textChanged(const QString &text);
    void on_eDownloadFolder_textChanged(const QString &text);

    void on_cFileVersioning_toggled(bool checked);
    void on_cOverlayIcons_toggled(bool checked);

    void on_bOpenProxySettings_clicked();
    void on_bOpenBandwidthSettings_clicked();

    // Security tab buttons
    void on_bExportMasterKey_clicked();
    void on_bChangePassword_clicked();


    // Footer buttons
    void on_bLogout_clicked();
    void on_bRestart_clicked();
    void on_bFullCheck_clicked();
#ifndef WIN32
    void on_bPermissions_clicked();
#endif
    void on_bSessionHistory_clicked();

#ifdef Q_OS_WINDOWS
    void on_cFinderIcons_toggled(bool checked);
#endif

#ifdef Q_OS_MACOS
    void onAnimationFinished();
    void initializeNativeUIComponents();
#endif


protected:
    void changeEvent(QEvent * event) override;

private:
    void loadSettings();
    void saveSyncSettings();
    void onCacheSizeAvailable();
    void onClearCache();
    void savingSyncs(bool completed, QObject *item);
    void syncsStateInformation(int state);
    QString excludeBySizeInfo();
    QString cacheDaysLimitInfo();
    void saveExcludeSyncNames();
    void updateNetworkTab();

    Ui::SettingsDialog *ui;
    MegaApplication *app;
    Preferences *preferences;
    Controller *controller;
    Model *model;
    mega::MegaApi *megaApi;
    HighDpiResize highDpiResize;
    QStringList syncNames;
    QStringList languageCodes;
    bool proxyOnly;
    QFutureWatcher<long long> cacheSizeWatcher;
    QFutureWatcher<long long> remoteCacheSizeWatcher;
    AccountDetailsDialog *accountDetailsDialog;
    std::unique_ptr<ProgressHelper> saveSettingsProgress;
    int loadingSettings;
    long long cacheSize;
    long long remoteCacheSize;
    bool hasDefaultUploadOption;
    bool hasDefaultDownloadOption;
    bool reloadUIpage;
    ThreadPool* mThreadPool;
    bool areSyncsDisabled; //Check if there are any sync disabled by any kind of error
    bool isSavingSyncsOnGoing;
    int debugCounter; // Easter Egg
    QPointer<ProxySettings> mProxySettingsDialog;

#ifdef Q_OS_MACOS
    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;

    std::unique_ptr<QMacToolBar> toolBar;
    std::unique_ptr<QMacToolBarItem> bGeneral;
    std::unique_ptr<QMacToolBarItem> bAccount;
    std::unique_ptr<QMacToolBarItem> bSyncs;
    std::unique_ptr<QMacToolBarItem> bImports;
    std::unique_ptr<QMacToolBarItem> bNetwork;
    std::unique_ptr<QMacToolBarItem> bSecurity;

    void animateSettingPage(int endValue, int duration = 150);
#endif

};

#endif // SETTINGSDIALOG_H
