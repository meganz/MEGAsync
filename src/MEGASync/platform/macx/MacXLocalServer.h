#ifndef MACXLOCALSERVER_H
#define MACXLOCALSERVER_H

#include "MacXLocalSocket.h"

class MacXLocalServerPrivate;
class MacXLocalServer : public QObject
{
    Q_OBJECT

public:
    MacXLocalServer();
    ~MacXLocalServer();

    bool listen(QString name);
    MacXLocalSocket* nextPendingConnection();
    bool hasPendingConnections();
    void appenPendingConnection(MacXLocalSocket* client);

signals:
    void newConnection();

private:
    QList<MacXLocalSocket*> pendingConnections;
    QScopedPointer<MacXLocalServerPrivate> serverPrivate;
};

#endif // MACXLOCALSERVER_H
