#include "MacXLocalSocketPrivate.h"
#include "ClientSide.h"

#include <QMutexLocker>

MacXLocalSocketPrivate::MacXLocalSocketPrivate(NSDistantObject<CommunicationProtocol> *extClient)
{
    this->extClient = extClient;
    client = [[ClientSide alloc] initWithLocalSocket:this];
    [extClient retain];
}

MacXLocalSocketPrivate::~MacXLocalSocketPrivate()
{
    [client setSocketPrivate:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:client];
    [extClient release];
    [client release];
}

void MacXLocalSocketPrivate::appendToBuffer(QByteArray info)
{
    QMutexLocker lock(&mBufferMutex);
    buf += info;
}

void MacXLocalSocketPrivate::clearBuffer()
{
    QMutexLocker lock(&mBufferMutex);
    buf.remove(0, buf.size());
}

void MacXLocalSocketPrivate::removeFromBuffer(int pos, int size)
{
    QMutexLocker lock(&mBufferMutex);
    buf.remove(pos, size);
}

QByteArray MacXLocalSocketPrivate::midFromBuffer(int pos, int size)
{
    QMutexLocker lock(&mBufferMutex);
    return buf.mid(pos, size);
}

int MacXLocalSocketPrivate::bufferSize()
{
    QMutexLocker lock(&mBufferMutex);
    return buf.size();
}

QByteArray MacXLocalSocketPrivate::getBuffer()
{
    QMutexLocker lock(&mBufferMutex);
    return buf;
}
