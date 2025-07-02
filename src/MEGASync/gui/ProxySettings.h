#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include "ConnectivityChecker.h"
#include "MegaApplication.h"
#include "MegaProgressCustomDialog.h"
#include "Preferences.h"

#include <QDialog>
#include <QRadioButton>

namespace Ui {
class ProxySettings;
}

class ProxySettings : public QDialog
{
    Q_OBJECT

public:
    explicit ProxySettings(MegaApplication* app, QWidget* parent = nullptr);
    ~ProxySettings();

private slots:
    void onProxyTestFinished(bool success);

    void on_bUpdate_clicked();
    void on_bCancel_clicked();

private:
    void initialize();
    void setManualMode(bool enabled);

    Ui::ProxySettings* mUi;
    MegaApplication* mApp;
    std::shared_ptr<Preferences> mPreferences;
    ConnectivityChecker* mConnectivityChecker;
    QPointer<MegaProgressCustomDialog> mProgressDialog;
    QRadioButton* mProxyAuto;
};

#endif // PROXYSETTINGS_H
