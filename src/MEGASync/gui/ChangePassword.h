#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include "megaapi.h"
#include "QTMegaRequestListener.h"

#include <QDialog>

namespace Ui {
class ChangePassword;
}

class ChangePassword : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget* parent = 0);
    QString newPassword();
    QString confirmNewPassword();
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* req, mega::MegaError* e);
    ~ChangePassword();

private:
    Ui::ChangePassword* mUi;
    mega::MegaApi* mMegaApi;
    mega::QTMegaRequestListener* mDelegateListener;

protected:
    void changeEvent(QEvent * event);

private slots:
    void on_bOk_clicked();

private:
    void show2FA(bool invalidCode);
};

#endif // CHANGEPASSWORD_H
