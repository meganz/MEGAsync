#ifndef CHANGE_PASSWORD_CONTROLLER_H
#define CHANGE_PASSWORD_CONTROLLER_H

#include "megaapi.h"

#include <QObject>

class ChangePasswordController: public QObject
{
    Q_OBJECT

public:
    explicit ChangePasswordController(QObject* parent = 0);
    void changePassword(QString password, bool signOutOtherDevices);
    void check2FA(QString pin);
    void onRequestFinish(mega::MegaRequest* req, mega::MegaError* e);

signals:
    void show2FA();
    void passwordChangeFailed(QString message);
    void passwordChangeSucceed(QString title, QString description, QString bannerText);
    void twoFAVerificationFailed();
    void passwordCheckFailed(QString error);

private:
    void emitPasswordChangeSucceed(bool signedOutOtherDevices);

    QString mPassword;
    bool mSignOutOtherDevices = false;
    mega::MegaApi* mMegaApi;
};

#endif
