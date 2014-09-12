#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QAuthenticator>

#include "AccountDetailsDialog.h"
#include "BindFolderDialog.h"
#include "Preferences.h"
#include "megaapi.h"

namespace Ui {
class SettingsDialog;
}

class MegaProgressDialog : public QProgressDialog
{
public:
    MegaProgressDialog(const QString & labelText, const QString & cancelButtonText, int minimum, int maximum, QWidget * parent = 0, Qt::WindowFlags f = 0);
protected:
    void reject();
    void closeEvent(QCloseEvent * event);
};

class MegaApplication;
class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(MegaApplication *app, bool proxyOnly=false, QWidget *parent = 0);
    ~SettingsDialog();
    void setProxyOnly(bool proxyOnly);
    void loadSettings();
    void refreshAccountDetails();
    void setUpdateAvailable(bool updateAvailable);

public slots:
    void stateChanged();
    void proxyStateChanged();
    void onCacheSizeAvailable();
    
private slots:
    void on_bAccount_clicked();

    void on_bSyncs_clicked();

    void on_bBandwidth_clicked();

    void on_bAdvanced_clicked();

    void on_bProxies_clicked();

    void on_bCancel_clicked();

    void on_bOk_clicked();

    void on_bHelp_clicked();

    void on_rProxyManual_clicked();

    void on_rProxyAuto_clicked();

    void on_rNoProxy_clicked();

    void on_bUpgrade_clicked();
	void on_bUpgradeBandwidth_clicked();

    void on_rNoLimit_clicked();

    void on_rLimit_clicked();

    void on_cProxyRequiresPassword_clicked();

    void on_bAdd_clicked();
    void on_bApply_clicked();
    void on_bDelete_clicked();

    void on_bUnlink_clicked();

    void on_tSyncs_doubleClicked(const QModelIndex &index);
    void on_bUploadFolder_clicked();

    void on_bAddName_clicked();
    void on_bDeleteName_clicked();
    void on_bClearCache_clicked();
    void onProxyTestTimeout();
    void onProxyTestFinished(QNetworkReply* reply);
    void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator* auth);
    void on_bUpdate_clicked();
    void on_bSyncChange_clicked();
    void on_bFullCheck_clicked();
    void on_bStorageDetails_clicked();

    void onAnimationFinished();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::SettingsDialog *ui;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
	bool syncsChanged;
    bool excludedNamesChanged;
    QStringList syncNames;
    QStringList languageCodes;
    bool proxyOnly;
    QFutureWatcher<long long> cacheSizeWatcher;
    QNetworkAccessManager *networkAccess;
    MegaProgressDialog *proxyTestProgressDialog;
    AccountDetailsDialog *accountDetailsDialog;

    QTimer proxyTestTimer;
    bool shouldClose;
    int modifyingSettings;

#ifdef __APPLE__
    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
#endif

    void loadSyncSettings();
    bool saveSettings();
    void updateAddButton();
};

#endif // SETTINGSDIALOG_H
