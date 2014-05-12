#ifndef NOTIFYSERVER_H
#define NOTIFYSERVER_H

#include "MegaApplication.h"
#include "sdk/megaapi.h"
#include "control/Preferences.h"

class NotifyServer: public QObject
{
    Q_OBJECT

 public:
    NotifyServer();
    virtual ~NotifyServer();
    void notifyItemChange(QString path);
    void notifySyncAdd(QString path);
    void notifySyncDel(QString path);

 protected:
    QLocalServer *m_localServer;

 public Q_SLOTS:
    void acceptConnection();
    void onClientDisconnected();
 private:
    MegaApplication *app;
    QString sockPath;
    QList<QLocalSocket *> m_clients;
    void sendToAll(const char *type, QString str);
};

#endif

