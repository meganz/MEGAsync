#include "NotifyServer.h"

#include <QLatin1String>
#include <sys/types.h>

#include <pwd.h>
#include <unistd.h>

using namespace mega;
using namespace std;

NotifyServer::NotifyServer():
    QObject(),
    mLocalServer(0)
{
    // construct local socket path
    mSockPath = MegaApplication::applicationDataPath() + QDir::separator() +
                QString::fromLatin1("notify.socket");

    // LOG_info << "Starting Notify server";

    // make sure previous socket file is removed
    QLocalServer::removeServer(mSockPath);

    mLocalServer = new QLocalServer(this);

    // start listening for new connections
    if (!mLocalServer->listen(mSockPath))
    {
        // XXX: failed to open local socket, retry ?
        // LOG_err << "Failed to listen()";
        return;
    }

    connect(mLocalServer, &QLocalServer::newConnection, this, &NotifyServer::acceptConnection);
    connect(this, &NotifyServer::sendToAll, this, &NotifyServer::doSendToAll);
}

NotifyServer::~NotifyServer()
{
    // Stop accepting new connections
    if (mLocalServer)
    {
        mLocalServer->close();
    }

    // Close and schedule deletion of all clients
    for (auto* client: mClients)
    {
        if (!client)
            continue;

        client->disconnect(this);
        client->close();
        client->deleteLater();
    }
    mClients.clear();

    // Remove socket file path
    QLocalServer::removeServer(mSockPath);
}

// a new connection is available
void NotifyServer::acceptConnection()
{
    while (mLocalServer && mLocalServer->hasPendingConnections())
    {
        QLocalSocket* client = mLocalServer->nextPendingConnection();

        // LOG_debug << "Incoming connection";
        if (!client)
        {
            return;
        }

        client->setParent(this);

        connect(client, &QLocalSocket::disconnected, this, &NotifyServer::onClientDisconnected);

        // send the list of current synced folders to the new client
        int localFolders = 0;
        SyncInfo* model = SyncInfo::instance();
        for (auto& syncSetting: model->getAllSyncSettings())
        {
            QString c =
                QDir::toNativeSeparators(QDir(syncSetting->getLocalFolder()).canonicalPath());
            if (!c.isEmpty() && syncSetting->isActive())
            {
                localFolders++;
                client->write(createPayload(NotifyType::SyncAdded, c.toUtf8().constData()));
            }
        }

        if (!localFolders)
        {
            // send an empty sync
            client->write(createPayload(NotifyType::SyncAdded, QLatin1String(".").latin1()));
        }

        mClients.append(client);
    }
}

// client disconnected
void NotifyServer::onClientDisconnected()
{
    QLocalSocket* client = qobject_cast<QLocalSocket*>(sender());
    if (!client)
        return;
    mClients.removeAll(client);
    client->deleteLater();

    // LOG_debug << "Client disconnected";
}

// send string to all connected clients
void NotifyServer::doSendToAll(const QByteArray& payload)
{
    for (QLocalSocket* socket: mClients)
    {
        if (socket && socket->state() == QLocalSocket::ConnectedState) {
            socket->write(payload);
            socket->flush();
        }
    }
}

QByteArray NotifyServer::createPayload(NotifyType type, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(type));
    payload.append(data);
    payload.append(static_cast<char>(NotifyType::EndLine));
    return payload;
}

void NotifyServer::notifyItemChange(const QString& localPath)
{
    emit sendToAll(createPayload(NotifyType::ItemChanged, localPath.toUtf8()));
}

void NotifyServer::notifySyncAdd(const QString& path)
{
    emit sendToAll(createPayload(NotifyType::SyncAdded, path.toUtf8()));
}

void NotifyServer::notifySyncDel(const QString& path)
{
    emit sendToAll(createPayload(NotifyType::SyncDeleted, path.toUtf8()));
}
