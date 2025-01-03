#ifndef VERIFYLOCKMESSAGE_H
#define VERIFYLOCKMESSAGE_H

#include "megaapi.h"

#include <QDialog>
#include <QMouseEvent>

#include <memory>

#ifdef __APPLE__
    #include "platform/macx/NativeMacPopover.h"
    #import <objc/runtime.h>
#else
    #include "LockedPopOver.h"
#endif

namespace Ui {
class VerifyLockMessage;
}

class VerifyLockMessage : public QDialog
{
    Q_OBJECT

public:

    explicit VerifyLockMessage(int lockStatus, bool isMainDialogAvailable = true, QWidget *parent = nullptr);
    ~VerifyLockMessage();

    void regenerateUI(int newStatus, bool force = false);

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError* e);

signals:
    void logout();
    void resendEmail();

private slots:
    void on_bLogout_clicked();
    void on_bResendEmail_clicked();

protected:
    mega::MegaApi *megaApi;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

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
