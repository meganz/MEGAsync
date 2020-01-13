#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>
#include <QNetworkProxy>
#include <QButtonGroup>
#include <ConnectivityChecker.h>

#include "AccountDetailsDialog.h"
#include "BindFolderDialog.h"
#include "SizeLimitDialog.h"
#include "LocalCleanScheduler.h"
#include "DownloadFromMegaDialog.h"
#include "MegaProgressCustomDialog.h"
#include "ChangePassword.h"
#include "Preferences.h"
#include "megaapi.h"
#include "HighDpiResize.h"

namespace Ui {
class SettingsDialog;
}

class MegaApplication;
class SettingsDialog : public QDialog, public IStorageObserver, public IBandwidthObserver, public IAccountObserver
{
    Q_OBJECT
    
public:
    enum {ACCOUNT_TAB = 0, SYNCS_TAB = 1, BANDWIDTH_TAB = 2, PROXY_TAB = 3, ADVANCED_TAB = 4};

    explicit SettingsDialog(MegaApplication *app, bool proxyOnly = false, QWidget *parent = 0);
    ~SettingsDialog();
    void setProxyOnly(bool proxyOnly);
    void setOverQuotaMode(bool mode);
    void loadSettings();
    void refreshAccountDetails();
    void setUpdateAvailable(bool updateAvailable);
    void openSettingsTab(int tab);
    void storageChanged();

public slots:
    void stateChanged();
    void fileVersioningStateChanged();
    void syncStateChanged(int state);
    void proxyStateChanged();
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();
    
private slots:
    void on_bAccount_clicked();

    void on_bSyncs_clicked();

    void on_bBandwidth_clicked();

    void on_bAdvanced_clicked();

    void on_bProxies_clicked();

    void on_bCancel_clicked();

    void on_bOk_clicked();

    void on_bHelp_clicked();

#ifndef __APPLE__
    void on_bHelpIco_clicked();
#endif

    void on_rProxyManual_clicked();

    void on_rProxyAuto_clicked();

    void on_rNoProxy_clicked();

    void on_bUpgrade_clicked();
    void on_bUpgradeBandwidth_clicked();

    void on_rUploadAutoLimit_clicked();
    void on_rUploadNoLimit_clicked();
    void on_rUploadLimit_clicked();

    void on_rDownloadNoLimit_clicked();
    void on_rDownloadLimit_clicked();

    void on_cProxyRequiresPassword_clicked();
#ifndef WIN32
    void on_bPermissions_clicked();
#endif
    void on_bAdd_clicked();
    void on_bApply_clicked();
    void on_bDelete_clicked();
    void on_bExcludeSize_clicked();
    void on_bLocalCleaner_clicked();

    void on_bUnlink_clicked();
    void on_bExportMasterKey_clicked();

    void on_tSyncs_doubleClicked(const QModelIndex &index);
    void on_bUploadFolder_clicked();
    void on_bDownloadFolder_clicked();

    void on_bAddName_clicked();
    void on_bDeleteName_clicked();
    void on_bClearCache_clicked();
    void on_bClearRemoteCache_clicked();
    void on_bClearFileVersions_clicked();
    void onProxyTestError();
    void onProxyTestSuccess();
    void on_bUpdate_clicked();
    void on_bFullCheck_clicked();
    void on_bStorageDetails_clicked();
    void on_lAccountImage_clicked();
    void on_bChangePassword_clicked();
    void on_bSendBug_clicked();

    void onAnimationFinished();

signals:
    void userActivity();

protected:
    void changeEvent(QEvent * event);
    QString getFormatString();
    QString getFormatLimitDays();

private:
    Ui::SettingsDialog *ui;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    HighDpiResize highDpiResize;
    bool syncsChanged;
    bool excludedNamesChanged;
    QStringList syncNames;
    QStringList languageCodes;
    bool proxyOnly;
    QFutureWatcher<long long> cacheSizeWatcher;
    QFutureWatcher<long long> remoteCacheSizeWatcher;
    MegaProgressCustomDialog *proxyTestProgressDialog;
    AccountDetailsDialog *accountDetailsDialog;
    bool shouldClose;
    int modifyingSettings;
    long long cacheSize;
    long long remoteCacheSize;
    long long fileVersionsSize;
    bool hasDefaultUploadOption;
    bool hasDefaultDownloadOption;
    bool hasUpperLimit;
    bool hasLowerLimit;
    long long upperLimit;
    long long lowerLimit;
    int upperLimitUnit;
    int lowerLimitUnit;
    bool sizeLimitsChanged;
    bool hasDaysLimit;
    int daysLimit;
    bool cleanerLimitsChanged;
    bool fileVersioningChanged;
    QButtonGroup downloadButtonGroup;
    QButtonGroup uploadButtonGroup;
    bool reloadUIpage;

#ifndef WIN32
    int folderPermissions;
    int filePermissions;
    bool permissionsChanged;
#endif
    int debugCounter;

#ifdef __APPLE__
    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
#endif

    void loadSizeLimits();
    int saveSettings();
    void onCacheSizeAvailable();
    void onClearCache();

public:
    void updateStorageElements();
    void updateBandwidthElements();
    void updateAccountElements();
    void loadSyncSettings();
    void updateUploadFolder();
    void updateDownloadFolder();
};

#endif // SETTINGSDIALOG_H
