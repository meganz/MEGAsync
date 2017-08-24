#include "MacXLocalSocket.h"
#include "MacXLocalSocketPrivate.h"
#include "megaapi.h"
#import <Cocoa/Cocoa.h>

using namespace mega;
using namespace std;

MacXLocalSocket::MacXLocalSocket(QObject *parent, MacXLocalSocketPrivate *clientSocketPrivate)
    : QObject(parent), socketPrivate(clientSocketPrivate)
{
    socketPrivate->socket = this;
}

MacXLocalSocket::~MacXLocalSocket()
{
}

qint64 MacXLocalSocket::readCommand(QByteArray *data)
{
    int currentPos = 0;
    if (!socketPrivate->buf.size())
    {
        return -1;
    }

    char opCommand = '\0';
    const char *ptr = socketPrivate->buf.constData();
    const char* end = ptr + socketPrivate->buf.size();

    if (ptr + sizeof(char) > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not op code");
        socketPrivate->buf.remove(0, socketPrivate->buf.size());
        return -1;
    }

    opCommand = *ptr;
    data->append(opCommand);
    ptr += sizeof(char);
    currentPos += sizeof(char);

    if (ptr + sizeof(char) > end || *ptr != ':')
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not first separator");
        socketPrivate->buf.remove(0, socketPrivate->buf.size());
        return -1;
    }
    ptr += sizeof(char);
    currentPos += sizeof(char);

    if (ptr + sizeof(int) > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not command length");
        socketPrivate->buf.remove(0, socketPrivate->buf.size());
        return -1;
    }

    int commandLength;
    memcpy(&commandLength, ptr, sizeof(int));
    ptr += sizeof(int);
    currentPos += sizeof(int);

    if (ptr + sizeof(char) > end || *ptr != ':')
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not second separator");
        socketPrivate->buf.remove(0, socketPrivate->buf.size());
        return -1;
    }
    ptr += sizeof(char);

    if (ptr + commandLength > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: file path too long");
        socketPrivate->buf.remove(0, socketPrivate->buf.size());
        return -1;
    }

    data->append(socketPrivate->buf.mid(currentPos, commandLength + 1));
    socketPrivate->buf.remove(0, commandLength + 3 + sizeof(int));

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Command from shell ext: %1")
                 .arg(QString::fromUtf8(data->constData(), data->size())).toUtf8().constData());

    return data->size();
}

qint64 MacXLocalSocket::writeData(const char *data, qint64 len)
{
    @try
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Sending data to shell ext: %1")
                     .arg(QString::fromUtf8(data, len)).toUtf8().constData());
        [socketPrivate->extClient send:[NSData dataWithBytes:(const void *)data length:sizeof(unsigned char)*len]];
        return len;
    }
    @catch(NSException *e)
    {
        emit disconnected();
        return -1;
    }
}
