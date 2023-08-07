#ifndef VERIFYLOCKMESSAGE_H
#define VERIFYLOCKMESSAGE_H

#include <QDialog>
#include <QMouseEvent>
#include <memory>
#include "megaapi.h"
#include "QTMegaRequestListener.h"

#ifdef __APPLE__
    #include "platform/macx/NativeMacPopover.h"
    #import <objc/runtime.h>
#else
    #include "LockedPopOver.h"
#endif

namespace Ui {
class VerifyLockMessage;
}

class VerifyLockMessage : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:

    explicit VerifyLockMessage(int lockStatus, bool isMainDialogAvailable = true, QWidget *parent = nullptr);
    ~VerifyLockMessage();

    void regenerateUI(int newStatus, bool force = false);

    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

signals:
    void logout();
    void resendEmail();

private slots:
    void on_bLogout_clicked();
    void on_bResendEmail_clicked();

protected:
    mega::MegaApi *megaApi;
    mega::QTMegaRequestListener *delegateListener;

    void mousePressEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);

private:
    Ui::VerifyLockMessage *m_ui;
    int m_lockStatus;
    bool m_haveMainDialog;

#ifdef __APPLE__
    NativeMacPopover mPopOver;
#else
    std::unique_ptr<LockedPopOver> mLockedPopOver{new LockedPopOver(this)};
#endif
};

#endif // VERIFYLOCKMESSAGE_H
