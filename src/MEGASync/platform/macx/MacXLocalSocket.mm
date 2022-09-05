#include "MacXLocalSocket.h"
#include "MacXLocalSocketPrivate.h"
#include "megaapi.h"
#include "MacXFunctions.h"
#import <Cocoa/Cocoa.h>

using namespace mega;
using namespace std;

MacXLocalSocket::MacXLocalSocket(MacXLocalSocketPrivate *clientSocketPrivate)
    : socketPrivate(clientSocketPrivate)
{
    socketPrivate->socket = this;
}

MacXLocalSocket::~MacXLocalSocket()
{
    delete socketPrivate;
}

qint64 MacXLocalSocket::readCommand(QByteArray *data)
{
    int currentPos = 0;
    if (!socketPrivate->bufferSize())
    {
        return -1;
    }

    char opCommand = '\0';
    const char *ptr = socketPrivate->getBuffer().constData();
    const char* end = ptr + socketPrivate->bufferSize();

    if (ptr + sizeof(char) > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not op code");
        socketPrivate->clearBuffer();
        return -1;
    }

    opCommand = *ptr;
    data->append(opCommand);
    ptr += sizeof(char);
    currentPos += sizeof(char);

    if (ptr + sizeof(char) > end || *ptr != ':')
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not first separator");
        socketPrivate->clearBuffer();
        return -1;
    }
    ptr += sizeof(char);
    currentPos += sizeof(char);

    if (ptr + sizeof(uint32_t) > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not command length");
        socketPrivate->clearBuffer();
        return -1;
    }

    uint32_t commandLength;
    memcpy(&commandLength, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    currentPos += sizeof(uint32_t);

    if (ptr + sizeof(char) > end || *ptr != ':')
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: Not second separator");
        socketPrivate->clearBuffer();
        return -1;
    }
    ptr += sizeof(char);

    if (ptr + commandLength > end)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error reading command from shell ext: file path too long");
        socketPrivate->clearBuffer();
        return -1;
    }

    data->append(socketPrivate->midFromBuffer(currentPos, (int)(commandLength + 1))); // + 1 is to copy the ':' character from the source string
    socketPrivate->removeFromBuffer(0, (int) (commandLength + 3 + sizeof(uint32_t))); // 3 = opCommand + 2 ':' separator characters

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Command from shell ext: %1")
                 .arg(QString::fromUtf8(data->constData(), data->size())).toUtf8().constData());

    return data->size();
}

bool MacXLocalSocket::writeData(const char *data, qint64 len)
{
    if (!len)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Skipping write of zero bytes");
        return true;
    }

    @try
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Sending data to shell ext: %1")
                     .arg(QString::fromUtf8(data, len)).toUtf8().constData());
        [socketPrivate->extClient send:[NSData dataWithBytes:(const void *)data length:sizeof(unsigned char)*len]];
        return true;
    }
    @catch(NSException *e)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("writeData to socket failed: %1").arg(fromNSString([e reason])).toUtf8().constData());
        return false;
    }
}
