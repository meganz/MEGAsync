#ifndef CHANGE_PASSWORD_CONTROLLER_H
#define CHANGE_PASSWORD_CONTROLLER_H

#include "megaapi.h"

#include <QObject>

class ChangePasswordController: public QObject
{
    Q_OBJECT

public:
    explicit ChangePasswordController(QObject* parent = 0);

    void requestChangePassword(QString password, QString confirmPassword);
    void onRequestFinish(mega::MegaRequest* req, mega::MegaError* e);

private:
    mega::MegaApi* mMegaApi;

private:
    void show2FA(bool invalidCode);
    QString mPassword;
};

#endif
