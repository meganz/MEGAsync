#ifndef CHANGE_PASSWORD_CONTROLLER_H
#define CHANGE_PASSWORD_CONTROLLER_H

#include "megaapi.h"

#include <QObject>

class ChangePasswordController: public QObject
{
    Q_OBJECT

public:
    explicit ChangePasswordController(QObject* parent = 0);
    void changePassword(QString password, QString confirmPassword);
    void check2FA(QString pin);
    void onRequestFinish(mega::MegaRequest* req, mega::MegaError* e);

signals:
    void show2FA();
    void passwordChangeFailed();
    void passwordChangeSucceed();
    void twoFAVerificationFailed();

private:
    QString mPassword;
    mega::MegaApi* mMegaApi;
};

#endif
