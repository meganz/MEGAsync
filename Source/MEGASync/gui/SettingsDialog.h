#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "BindFolderDialog.h"
#include "Preferences.h"
#include "sdk/megaapi.h"

namespace Ui {
class SettingsDialog;
}

class MegaApplication;
class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(MegaApplication *app, bool proxyOnly=false, QWidget *parent = 0);
    ~SettingsDialog();
    void setProxyOnly(bool proxyOnly);

public slots:
    void stateChanged();
    
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

protected:
    void changeEvent(QEvent * event);

private:
    Ui::SettingsDialog *ui;
    MegaApplication *app;
    Preferences *preferences;
    MegaApi *megaApi;
	bool syncsChanged;
    bool excludedNamesChanged;
    QStringList syncNames;
    QStringList languageCodes;
    bool proxyOnly;

    void loadSyncSettings();
    void loadSettings();
    void saveSettings();
    void updateAddButton();
};

#endif // SETTINGSDIALOG_H
