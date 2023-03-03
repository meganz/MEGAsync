#ifndef MACXLOCALSERVER_H
#define MACXLOCALSERVER_H

#include "MacXLocalSocket.h"

#include <QPointer>

class MacXLocalServerPrivate;
class MacXLocalServer : public QObject
{
    Q_OBJECT

public:
    MacXLocalServer(QObject *parent);
    ~MacXLocalServer();

    bool listen(QString name);

signals:
    void newConnection(QPointer<MacXLocalSocket> client);

private:
    MacXLocalServerPrivate *serverPrivate;
    bool listening;
};

#endif // MACXLOCALSERVER_H
