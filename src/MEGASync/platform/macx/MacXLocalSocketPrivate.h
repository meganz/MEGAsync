#ifndef MACXLOCALSOCKETPRIVATE_H
#define MACXLOCALSOCKETPRIVATE_H

#include <QMutex>

#include "Protocol.h"
#include "MacXLocalSocket.h"

class MacXLocalSocketPrivate
{
public:
    MacXLocalSocket *socket;
    NSDistantObject<CommunicationProtocol> *extClient;
    id client;

    MacXLocalSocketPrivate(NSDistantObject<CommunicationProtocol> *extClient);
    ~MacXLocalSocketPrivate();

    void appendToBuffer(QByteArray info);
    void clearBuffer();
    void removeFromBuffer(int pos, int size);
    QByteArray midFromBuffer(int pos, int size);
    int bufferSize();
    QByteArray getBuffer();

private:
    QByteArray buf;
    QMutex mBufferMutex;
};

#endif // MACXLOCALSOCKETPRIVATE_H
