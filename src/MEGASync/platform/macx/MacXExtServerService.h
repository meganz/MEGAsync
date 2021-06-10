#ifndef MACXEXTSERVERSERVICE_H
#define MACXEXTSERVERSERVICE_H

#include <QObject>
#include <QPointer>
#include "platform/macx/MacXExtServer.h"

class MacXExtServerService : public QObject
{
    Q_OBJECT
public:
    MacXExtServerService(MegaApplication *receiver);
    ~MacXExtServerService();

signals:
    void syncAdd(QString path, QString syncName);
    void syncDel(QString path, QString syncName);
    void allClients(int op);
    void itemChange(QString localPath, int newState);

private:
    QThread threadExtServer;
    QPointer<MacXExtServer> extServer;
};

#endif // MACXEXTSERVERSERVICE_H
