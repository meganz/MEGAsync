#include "ExtServer.h"
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "control/Utilities.h"

using namespace mega;
using namespace std;

ExtServer::ExtServer(MegaApplication *app): QObject(),
    m_localServer(0)
{
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), app, SLOT(shellUpload(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), app, SLOT(shellExport(QQueue<QString>)),Qt::QueuedConnection);

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
    qDeleteAll(m_clients);
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

    qint64 len;
    char buf[1024];
    while ((len = client->readLine(buf, sizeof(buf))) > 0) {
        const char *out = GetAnswerToRequest(buf);
        if (out) {
            qint64 len = client->write(out);
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
            if (!ok)
            {
                break;
            }

            int numFolders = parameters[2].toInt(&ok);
            if (!ok)
            {
                break;
            }

            QString actionString;
            switch (stringId)
            {
                case STRING_UPLOAD:
                    actionString = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
                    break;
                case STRING_GETLINK:
                    actionString = QCoreApplication::translate("ShellExtension", "Get MEGA link");
                    break;
                case STRING_SHARE:
                    actionString = QCoreApplication::translate("ShellExtension", "Share with a MEGA user");
                    break;
                case STRING_SEND:
                    actionString = QCoreApplication::translate("ShellExtension", "Send to a MEGA user");
                    break;
            }

            QString sNumFiles;
            if (numFiles == 1)
            {
                sNumFiles = QCoreApplication::translate("ShellExtension", "1 file");
            }
            else if (numFiles > 1)
            {
                sNumFiles = QCoreApplication::translate("ShellExtension", "%1 files").arg(numFiles);
            }

            QString sNumFolders;
            if (numFolders == 1)
            {
                sNumFolders = QCoreApplication::translate("ShellExtension", "1 folder");
            }
            else if (numFolders > 1)
            {
                sNumFolders = QCoreApplication::translate("ShellExtension", "%1 folders").arg(numFolders);
            }

            QString fullString;
            if (numFiles && numFolders)
            {
                fullString = QCoreApplication::translate("ShellExtension", "%1 (%2, %3)")
                        .arg(actionString).arg(sNumFiles).arg(sNumFolders);
            }
            else if (numFiles && !numFolders)
            {
                fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)")
                        .arg(actionString).arg(sNumFiles);
            }
            else if (!numFiles && numFolders)
            {
                fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)")
                        .arg(actionString).arg(sNumFolders);
            }

            strncpy(out, fullString.toUtf8().constData(), BUFSIZE);
            break;
        }
        case 'F':
        {
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                //LOG_debug << "Adding file to upload queue";
                uploadQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            break;
        }
        case 'L':
        {
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                exportQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            break;
        }
        // get the state of an object
        case 'P':
        {
            int state = MegaApi::STATE_NONE;
            if (!Preferences::instance()->overlayIconsDisabled())
            {
                string tmpPath(content);
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
        case 'I':
        default:
            break;
    }

    return out;
}
