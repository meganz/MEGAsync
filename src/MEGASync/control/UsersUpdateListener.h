#ifndef USERSUPDATELISTENER_H
#define USERSUPDATELISTENER_H

#include "megaapi.h"

#include <QList>
#include <QMutex>
#include <QObject>
#include <QPointer>

class UsersUpdateListener: public QObject, public mega::MegaListener
{
    Q_OBJECT

public:
    UsersUpdateListener() = default;
    virtual ~UsersUpdateListener() = default;

    void onUsersUpdate(mega::MegaApi*, mega::MegaUserList* userList) override;

signals:
    void userEmailUpdated(mega::MegaHandle userHandle, const QString& newEmail);
};

#endif // USERSUPDATELISTENER_H
