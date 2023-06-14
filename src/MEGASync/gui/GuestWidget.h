#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"
#include "gui/MegaInfoMessage.h"

#include <QPointer>
#include <QWidget>
#include <QMenu>
#include <QPushButton>

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

private slots:
    void fetchNodesAfterBlockCallbak();

private:
    Ui::GuestWidget *ui;
    bool mSSLSecureConnectionFailed;
    bool incorrectCredentialsMessageReceived = false;

    void showSSLSecureConnectionErrorMessage(mega::MegaRequest* request) const;
};

#endif // GUESWIDGET_H
