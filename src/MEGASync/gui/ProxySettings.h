#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include <QDialog>

#include "Preferences.h"
#include "MegaProgressCustomDialog.h"
#include "MegaApplication.h"

namespace Ui {
class ProxySettings;
}

class ProxySettings : public QDialog
{
    Q_OBJECT

public:
    explicit ProxySettings(MegaApplication *app, QWidget *parent = nullptr);
    ~ProxySettings();

private slots:
    void onProxyTestError();
    void onProxyTestSuccess();

    void on_rProxyManual_clicked();
    void on_rProxyAuto_clicked();
    void on_rNoProxy_clicked();
    void on_cProxyRequiresPassword_clicked();

    void on_applyButton_clicked();

private:
    void initialize();
    Ui::ProxySettings *ui;

    MegaApplication *mApp;
    Preferences *preferences;
    MegaProgressCustomDialog *proxyTestProgressDialog;
};

#endif // PROXYSETTINGS_H
