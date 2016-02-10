#include "NotifyServer.h"
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "control/Utilities.h"

using namespace mega;

NotifyServer::NotifyServer(): QObject(),
    m_localServer(0)
{
    // construct local socket path
    sockPath = MegaApplication::applicationDataPath() + QDir::separator() + QString::fromAscii("notify.socket");

    //LOG_info << "Starting Notify server";

    // make sure previous socket file is removed
    QLocalServer::removeServer(sockPath);

    m_localServer = new QLocalServer(this);

    // start listening for new connections
    if (!m_localServer->listen(sockPath)) {
        // XXX: failed to open local socket, retry ?
        //LOG_err << "Failed to listen()";
        return;
    }

    connect(this, SIGNAL(sendToAll(const char *, QString )), this, SLOT(doSendToAll(const char *, QString)));
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

NotifyServer::~NotifyServer()
{
    qDeleteAll(m_clients);
    QLocalServer::removeServer(sockPath);
    m_localServer->close();
    delete m_localServer;
}

// a new connection is available
void NotifyServer::acceptConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket *client = m_localServer->nextPendingConnection();

        //LOG_debug << "Incoming connection";
        if (!client)
        {
            return;
        }

        connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

        // send the list of current synced folders to the new client
        int localFolders = 0;
        Preferences *preferences = Preferences::instance();
        for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            QString c = QDir::toNativeSeparators(QDir(preferences->getLocalFolder(i)).canonicalPath());
            if (!c.size() || !preferences->isFolderActive(i))
            {
                continue;
            }

            localFolders++;
            client->write("A");
            client->write(c.toUtf8().constData());
            client->write("\n");
        }

        if (!localFolders)
        {
            // send an empty sync
            client->write("A");
            client->write(".");
            client->write("\n");
        }

        m_clients.append(client);
    }
}

// client disconnected
void NotifyServer::onClientDisconnected()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client)
        return;
    m_clients.removeAll(client);
    client->deleteLater();

    //LOG_debug << "Client disconnected";
}

// send string to all connected clients
void NotifyServer::doSendToAll(const char *type, QString str)
{
    foreach(QLocalSocket *socket, m_clients)
        if (socket && socket->state() == QLocalSocket::ConnectedState) {
            socket->write(type);
            socket->write(str.toUtf8().constData());
            socket->write("\n");
            socket->flush();
        }
}

void NotifyServer::notifyItemChange(QString path)
{
    emit sendToAll("P", path);
}

void NotifyServer::notifySyncAdd(QString path)
{
    emit sendToAll("A", path);
}

void NotifyServer::notifySyncDel(QString path)
{
    emit sendToAll("D", path);
}

