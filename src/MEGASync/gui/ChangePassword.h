#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>
#include "megaapi.h"
#include "QTMegaRequestListener.h"

namespace Ui {
class ChangePassword;
}

class ChangePassword : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget *parent = 0);
    QString newPassword();
    QString confirmNewPassword();
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    ~ChangePassword();

private:
    Ui::ChangePassword *ui;
    mega::QTMegaRequestListener *delegateListener;
    mega::MegaApi *megaApi;

protected:
    void changeEvent(QEvent * event);

private slots:
    void on_bOk_clicked();
};

#endif // CHANGEPASSWORD_H
