#include "ExtServer.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "CommonMessages.h"
#include "control/Utilities.h"

using namespace mega;
using namespace std;

ExtServer::ExtServer(MegaApplication *app): QObject(),
    m_localServer(0)
{
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), app, SLOT(shellUpload(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), app, SLOT(shellExport(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(viewOnMega(QByteArray, bool)), app, SLOT(shellViewOnMega(QByteArray, bool)), Qt::QueuedConnection);


    // construct local socket path
    sockPath = MegaApplication::applicationDataPath() + QDir::separator() + QString::fromAscii("mega.socket");

    //LOG_info << "Starting Ext server";

    // make sure previous socket file is removed
    QLocalServer::removeServer(sockPath);

    m_localServer = new QLocalServer(this);

    // start listening for new connections
    if (!m_localServer->listen(sockPath)) {
        // XXX: failed to open local socket, retry ?
        //LOG_err << "Failed to listen()";
        return;
    }

    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()),Qt::QueuedConnection);
}

ExtServer::~ExtServer()
{
    for (auto client : m_clients)
    {
        client->deleteLater();
    }

    QLocalServer::removeServer(sockPath);
    m_localServer->close();
    delete m_localServer;
}

// a new connection is available
void ExtServer::acceptConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket *client = m_localServer->nextPendingConnection();

        //LOG_debug << "Incoming connection";
        if (!client)
            return;

        connect(client, SIGNAL(readyRead()), this, SLOT(onClientData()));
        connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

        m_clients.append(client);
    }
}

// client disconnected
void ExtServer::onClientDisconnected()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client)
        return;
    m_clients.removeAll(client);
    client->deleteLater();

    //LOG_debug << "Client disconnected";
}

// client sends some data
void ExtServer::onClientData()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client)
        return;

    if (m_clients.indexOf(client) == -1 ) {
        //LOG_err << "QLocalSocket not found !";
        return;
    }

    char buf[1024];
    while (client->readLine(buf, sizeof(buf)) > 0) {
        const char *out = GetAnswerToRequest(buf);
        if (out) {
            client->write(out);
            client->write("\n");
        }
    }
}

#define BUFSIZE 1024
#define RESPONSE_DEFAULT    "9"
#define RESPONSE_ERROR      "0"
#define RESPONSE_SYNCED     "1"
#define RESPONSE_PENDING    "2"
#define RESPONSE_SYNCING    "3"
// parse incoming request and send response back to client
const char *ExtServer::GetAnswerToRequest(const char *buf)
{
    char c = buf[0];
    const char *content = buf+2;
    static char out[BUFSIZE];

    strncpy(out, RESPONSE_DEFAULT, BUFSIZE);

    switch(c)
    {
        // send translated string
        case 'T':
        {
            if (strlen(buf) < 3)
            {
                break;
            }

            bool ok;
            QStringList parameters = QString::fromAscii(content).split(QChar::fromAscii(':'));

            if (parameters.size() != 3)
            {
                break;
            }

            int stringId = parameters[0].toInt(&ok);
            if (!ok)
            {
                break;
            }

            int numFiles = parameters[1].toInt(&ok);
            if (!ok || numFiles < 0)
            {
                break;
            }

            int numFolders = parameters[2].toInt(&ok);
            if (!ok || numFolders < 0)
            {
                break;
            }

            QString actionString = getActionName(stringId);
            QString fullString = CommonMessages::createShellExtensionActionLabel(actionString, numFiles, numFolders);
            strncpy(out, fullString.toUtf8().constData(), BUFSIZE);
            break;
        }
        case 'F':
        {
            addToQueue(uploadQueue, content);
            break;
        }
        case 'L':
        {
            addToQueue(exportQueue, content);
            break;
        }
        // get the state of an object
        case 'P':
        {
            int state = MegaApi::STATE_NONE;
            string scontent(content);
            size_t possep = scontent.find((char)0x1C);
            bool forceGetState = (possep != string::npos) && ((possep + 1) < scontent.size()) && scontent.at(possep+1) == '1';

            if (forceGetState || !Preferences::instance()->overlayIconsDisabled() )
            {
                string tmpPath = scontent.substr(0,possep);
                state = ((MegaApplication *)qApp)->getMegaApi()->syncPathState(&tmpPath);
            }

            switch(state)
            {
                case MegaApi::STATE_SYNCED:
                    strncpy(out, RESPONSE_SYNCED, BUFSIZE);
                    break;
                case MegaApi::STATE_SYNCING:
                    strncpy(out, RESPONSE_SYNCING, BUFSIZE);
                    break;
                case MegaApi::STATE_PENDING:
                    strncpy(out, RESPONSE_PENDING, BUFSIZE);
                    break;
                case MegaApi::STATE_NONE:
                case MegaApi::STATE_IGNORED:
                default:
                    strncpy(out, RESPONSE_DEFAULT, BUFSIZE);
            }
            break;
        }
        case 'E':
        {
            clearQueues();
            break;
        }
        case 'V': //View on MEGA
        {
            viewOnMega(content);
            break;
        }
        case 'R': //Open pRevious versions
        {
            viewOnMega(content);
            break;
        }
        case 'H': //Has previous versions? (still unsupported)
        {
            strncpy(out, "0", BUFSIZE);
            break;
        }
        case 'I':
        default:
            break;
    }

    return out;
}

QString ExtServer::getActionName(const int actionId)
{
    QString name;
    switch (actionId)
    {
        case STRING_UPLOAD:
            name = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
            break;
        case STRING_GETLINK:
            name = QCoreApplication::translate("ShellExtension", "Get MEGA link");
            break;
        case STRING_SHARE:
            name = QCoreApplication::translate("ShellExtension", "Share with a MEGA user");
            break;
        case STRING_SEND:
            name = QCoreApplication::translate("ShellExtension", "Send to a MEGA user");
            break;
        case STRING_VIEW_ON_MEGA:
            name = QCoreApplication::translate("ShellExtension", "View on MEGA");
            break;
        case STRING_VIEW_VERSIONS:
            name = QCoreApplication::translate("ShellExtension", "View previous versions");
            break;
    }
    return name;
}

void ExtServer::addToQueue(QQueue<QString> &queue, const char *content)
{
    const QString filePath = QString::fromUtf8(content);
    const QFileInfo file(filePath);
    if (file.exists())
    {
        queue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
    }
}

void ExtServer::clearQueues()
{
    if (!uploadQueue.isEmpty())
    {
        emit newUploadQueue(uploadQueue);
        uploadQueue.clear();
    }

    if (!exportQueue.isEmpty())
    {
        emit newExportQueue(exportQueue);
        exportQueue.clear();
    }
}

void ExtServer::viewOnMega(const char *content)
{
    const QString filePath = QString::fromUtf8(content);
    const QFileInfo file(filePath);
    if (file.exists())
    {
        emit viewOnMega(filePath.toUtf8(), false);
    }
}
