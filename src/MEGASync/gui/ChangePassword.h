#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include "megaapi.h"

#include <QDialog>

namespace Ui {
class ChangePassword;
}

class ChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget* parent = 0);
    QString newPassword();
    QString confirmNewPassword();
    void onRequestFinish(mega::MegaRequest* req, mega::MegaError* e);
    ~ChangePassword();

private:
    Ui::ChangePassword* mUi;
    mega::MegaApi* mMegaApi;

protected:
    void changeEvent(QEvent * event);

private slots:
    void on_bOk_clicked();

private:
    void show2FA(bool invalidCode);
};

#endif // CHANGEPASSWORD_H
