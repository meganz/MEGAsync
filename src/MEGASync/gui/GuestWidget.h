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

    explicit GuestWidget(QWidget *parent = 0);

    ~GuestWidget();

    virtual void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest *request);

#ifdef __APPLE__
    void resetFocus();
#endif

signals:
    void forwardAction(int action);

private slots:
    void on_bLogin_clicked();
    void on_bCreateAccount_clicked();
    void on_bSettings_clicked();
    void on_bForgotPassword_clicked();
    void on_bCancel_clicked();

private:
    Ui::GuestWidget *ui;
    MegaApplication *app;

protected:
    mega::QTMegaRequestListener *delegateListener;
    mega::MegaApi *megaApi;
    Preferences *preferences;
    bool closing;
    bool loggingStarted;

    void page_login();
    void page_progress();
    void page_settingUp();
    void page_logout();
};

#endif // GUESWIDGET_H
