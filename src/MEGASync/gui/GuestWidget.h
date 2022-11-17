#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"
#include "gui/MegaInfoMessage.h"

#include <memory>

namespace Ui {
class GuestWidget;
}

class MegaApplication;
class GuestWidget : public QWidget, public mega::MegaRequestListener
{
    Q_OBJECT

public:

    enum GuestWidgetState {
        //block states are on top of these
        NONE = -1,
        LOGOUT = 0,
        LOGGEDIN = 1,
        LOGIN = 2,
        PROGRESS = 3,
        SETTINGUP = 4,
        LOGIN2FA = 5,
    };

    explicit GuestWidget(QWidget *parent = 0);
    GuestWidget(mega::MegaApi* megaApi, QWidget *parent = 0);

    ~GuestWidget();

    void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest *request) override;

    void resetFocus();

    void disableListener();
    void enableListener();
    void initialize();

    void setBlockState(int lockType);

    void setTexts(const QString& s1, const QString& s2);
    std::pair<QString, QString> getTexts();

signals:
    void forwardAction(int action);
    void onPageLogin();

private slots:
    void on_bLogin_clicked();
    void on_bCreateAccount_clicked();
    void on_bSettings_clicked();
    void on_bForgotPassword_clicked();
    void on_bCancel_clicked();
    void on_bVerifySMSLogout_clicked();
    void on_bVerifyEmailLogout_clicked();
    void on_bVerifyEmail_clicked();
    void on_bVerifySMS_clicked();
    void on_bWhyAmIseen_clicked();
    void fetchNodesAfterBlockCallbak();
    void connectToSetupWizard();
    void onSetupWizardPageChanged(int page);
    void on_bLogin2FaNext_clicked();
    void on_bLoging2FaCancel_clicked();
    void on_bLogin2FaHelp_clicked();
    void hideLoginError();
    void hide2FaLoginError();

private:
    Ui::GuestWidget *ui;
    MegaApplication *app;
    QString mEmail, mPassword;
    bool mSSLSecureConnectionFailed;
    bool incorrectCredentialsMessageReceived = false;

    GuestWidgetState state = GuestWidgetState::NONE;
    void resetPageAfterBlock();
    void showLoginError(const QString& errorMessage) const;
    void showLogin2FaError() const;
    void showSSLSecureConnectionErrorMessage(mega::MegaRequest* request) const;


protected:
    mega::QTMegaRequestListener *delegateListener;
    mega::MegaApi *megaApi;
    std::shared_ptr<Preferences> preferences;
    bool closing;
    bool loggingStarted;

    std::unique_ptr<MegaInfoMessage> whyAmISeeingThisDialog;

    void page_login();
    void page_fetchnodes();
    void page_progress();
    void page_settingUp();
    void page_logout();
    void page_lockedEmailAccount();
    void page_lockedSMSAccount();
    void page_login2FA();

    void reset_UI_props();

    void changeEvent(QEvent * event) override;
};

#endif // GUESWIDGET_H
