#include "MacXExtServerService.h"

MacXExtServerService::MacXExtServerService(MegaApplication *receiver)
{
    extServer = new MacXExtServer(receiver);
    extServer->moveToThread(&threadExtServer);
    connect(&threadExtServer, &QThread::finished, extServer, &QObject::deleteLater);
    connect(this, &MacXExtServerService::syncAdd, extServer, &MacXExtServer::notifySyncAdd, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::syncDel, extServer, &MacXExtServer::notifySyncDel, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::allClients, extServer, &MacXExtServer::notifyAllClients, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::itemChange, extServer, &MacXExtServer::notifyItemChange, Qt::QueuedConnection);

    threadExtServer.start();
}

MacXExtServerService::~MacXExtServerService()
{
    threadExtServer.quit();
    threadExtServer.wait();
}
