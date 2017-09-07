#ifndef MACXLOCALSOCKETPRIVATE_H
#define MACXLOCALSOCKETPRIVATE_H

#include "Protocol.h"
#include "MacXLocalSocket.h"

class MacXLocalSocketPrivate
{
public:
    MacXLocalSocket *socket;
    NSDistantObject<CommunicationProtocol> *extClient;
    id client;
    QByteArray buf;

    MacXLocalSocketPrivate(NSDistantObject<CommunicationProtocol> *extClient);
    ~MacXLocalSocketPrivate();
};

#endif // MACXLOCALSOCKETPRIVATE_H
