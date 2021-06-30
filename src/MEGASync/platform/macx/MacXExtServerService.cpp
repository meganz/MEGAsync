#include "MacXExtServerService.h"

MacXExtServerService::MacXExtServerService(MegaApplication *receiver)
{
    mExtServer = new MacXExtServer(receiver);
    mExtServer->moveToThread(&mThreadExtServer);
    connect(&mThreadExtServer, &QThread::finished, mExtServer, &QObject::deleteLater);
    connect(this, &MacXExtServerService::syncAdd, mExtServer, &MacXExtServer::notifySyncAdd, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::syncDel, mExtServer, &MacXExtServer::notifySyncDel, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::allClients, mExtServer, &MacXExtServer::notifyAllClients, Qt::QueuedConnection);
    connect(this, &MacXExtServerService::itemChange, mExtServer, &MacXExtServer::notifyItemChange, Qt::QueuedConnection);

    mThreadExtServer.start();
}

MacXExtServerService::~MacXExtServerService()
{
    mThreadExtServer.quit();
    mThreadExtServer.wait();
}
