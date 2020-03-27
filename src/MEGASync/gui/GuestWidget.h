#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"

namespace Ui {
class GuestWidget;
}

class MegaApplication;
class GuestWidget : public QWidget, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum {
        INITIAL_CLICKED = 0,
        CREATE_ACCOUNT_CLICKED = 1,
        LOGIN_CLICKED = 2,
        CONFIG_MODE = 3
    };

    enum GuestWidgetState {
        //block states are on top of these
        NONE = -1,
        LOGOUT = 0,
        LOGGEDIN = 1,
        LOGIN = 2,
        PROGRESS = 3,
        SETTINGUP = 4,
    };

    explicit GuestWidget(QWidget *parent = 0);

    ~GuestWidget();

    virtual void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest *request);

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
    void fetchNodesAfterBlockCallbak();

private:
    Ui::GuestWidget *ui;
    MegaApplication *app;

    GuestWidgetState state = GuestWidgetState::NONE;
    void resetPageAfterBlock();


protected:
    mega::QTMegaRequestListener *delegateListener;
    mega::MegaApi *megaApi;
    Preferences *preferences;
    bool closing;
    bool loggingStarted;

    void page_login();
    void page_fetchnodes();
    void page_progress();
    void page_settingUp();
    void page_logout();
    void page_lockedEmailAccount();
    void page_lockedSMSAccount();

    void changeEvent(QEvent * event);
};

#endif // GUESWIDGET_H
