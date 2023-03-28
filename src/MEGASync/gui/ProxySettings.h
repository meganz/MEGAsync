#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include <QDialog>

#include "Preferences.h"
#include "MegaProgressCustomDialog.h"
#include "MegaApplication.h"
#include "ConnectivityChecker.h"

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
    void onProxyTestError();
    void onProxyTestSuccess();

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
};

#endif // PROXYSETTINGS_H
