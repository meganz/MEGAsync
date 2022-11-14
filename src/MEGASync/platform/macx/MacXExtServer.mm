#include "MacXExtServer.h"
#include <assert.h>
#include "CommonMessages.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;
using namespace std;

MacXExtServer::MacXExtServer(MegaApplication *app)
{
    sockPath = QString::fromUtf8("T9RH74Y7L9.mega.mac.socket");
    m_localServer = new MacXLocalServer();
    if (!m_localServer->listen(sockPath))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Failed to initialize the shell dispatcher");
        delete m_localServer;
        m_localServer = NULL;
        return;
    }

    connect(this, SIGNAL(sendToAll(QByteArray)), this, SLOT(doSendToAll(QByteArray)));
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), app, SLOT(shellUpload(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), app, SLOT(shellExport(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(viewOnMega(QByteArray, bool)), app, SLOT(shellViewOnMega(QByteArray, bool)),Qt::QueuedConnection);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

MacXExtServer::~MacXExtServer()
{
    for (auto client : m_clients)
    {
        client->deleteLater();
    }

    m_clients.clear();
    delete m_localServer;
}

void MacXExtServer::acceptConnection()
{
    while (m_localServer->hasPendingConnections())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Received connection from the shell ext");
        MacXLocalSocket *client = m_localServer->nextPendingConnection();
        if (!client)
        {
            continue;
        }

        connect(client, &MacXLocalSocket::dataReady, this, &MacXExtServer::onClientData);
        m_clients.append(client);

        // send the list of current synced folders to the new client
        SyncInfo *model = SyncInfo::instance();
        for (auto syncSetting : model->getAllSyncSettings())
        {
            QString syncPath = QDir::toNativeSeparators(QDir(syncSetting->getLocalFolder()).canonicalPath());
            if (!syncPath.size() || syncSetting->getRunState() != MegaSync::RUNSTATE_RUNNING)
            {
                continue;
            }

            QString message = QString::fromUtf8("A:") + syncPath
                    + QChar::fromAscii(':') + syncSetting->name(true);
            if(!client->writeData(message.toUtf8().constData(), message.length()))
            {
                clientDisconnected(client);
            }

        }        
    }
}

void MacXExtServer::onClientData()
{
    MacXLocalSocket *client = qobject_cast<MacXLocalSocket *>(sender());
    if (!client)
    {
        return;
    }

    if (m_clients.indexOf(client) == -1)
    {
        return;
    }

    qint64 len;
    QByteArray buf;
    QByteArray response;
    while ((len = client->readCommand(&buf)) > 0)
    {
        bool shouldRespond = GetAnswerToRequest(buf.constData(), &response);
        if (shouldRespond)
        {
            buf.append(':');
            buf.append(response);
            if(!client->writeData(buf.constData(), buf.size()))
            {
                clientDisconnected(client);
            }
        }
        buf.clear();
        response.clear();
    }
}

void MacXExtServer::clientDisconnected(QPointer<MacXLocalSocket> client)
{
    if (!client)
    {
        return;
    }
    m_clients.removeAll(client);
    client->deleteLater();
}

#define RESPONSE_DEFAULT    "9"
#define RESPONSE_ERROR      "0"
#define RESPONSE_SYNCED     "1"
#define RESPONSE_PENDING    "2"
#define RESPONSE_SYNCING    "3"
#define RESPONSE_IGNORED    "4"
bool MacXExtServer::GetAnswerToRequest(const char *buf, QByteArray *response)
{
    if (!buf || !response)
    {
        return false;
    }

    char c = buf[0];
    const char *content = buf + 2;
    response->clear();
    switch (c)
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

            if (!numFiles && !numFolders)
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
                default:
                    return false;
            }

            QString fullString = CommonMessages::createShellExtensionActionLabel(actionString, numFiles, numFolders);
            response->append(fullString.toUtf8().constData());
            return true;
        }
        case 'F':
        {
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Adding file to upload queue");
                uploadQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            return false;
        }
        case 'L':
        {
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Adding file to export queue");
                exportQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            return false;
        }
        // get the state of an object
        case 'P':
        {
            std::string tmpPath(content);
            int state = ((MegaApplication *)qApp)->getMegaApi()->syncPathState(&tmpPath);
            switch(state)
            {
                case MegaApi::STATE_SYNCED:
                    response->append(RESPONSE_SYNCED);
                    break;
                case MegaApi::STATE_SYNCING:
                    response->append(RESPONSE_SYNCING);
                    break;
                case MegaApi::STATE_PENDING:
                    response->append(RESPONSE_PENDING);
                    break;
                case MegaApi::STATE_NONE:
                case MegaApi::STATE_IGNORED:
                default:
                    return false;
            }

            response->append(":");
            if (Preferences::instance()->overlayIconsDisabled()) // Respond to extension to not show badges
            {
                response->append("0");
            }
            else // Respond to extension to show badges
            {
                response->append("1");
            }

            return true;
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
            return false;
        }
        case 'O': // Open local path
        {
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(filePath));
            }
            return false;
        }
        case 'V': // View node at MEGA cloud
        {
            QByteArray filePath = QByteArray(content, strlen(content) + 1);
            QFileInfo file(QString::fromUtf8(content));
            if (file.exists())
            {
                emit viewOnMega(filePath, false);
            }
            return false;
        }
        case 'R': // Open previous versions
        {
            QByteArray filePath = QByteArray(content, strlen(content) + 1);
            QFileInfo file(QString::fromUtf8(content));
            if (file.exists())
            {
                emit viewOnMega(filePath, true);
            }
            return false;
        }
        default:
            break;
    }
    return false;
}

void MacXExtServer::doSendToAll(QByteArray str)
{
    foreach(auto client, m_clients)
    {
        if (client)
        {
            if(!client->writeData(str.constData(), str.size()))
            {
                clientDisconnected(client);
            }
        }
    }
}

void MacXExtServer::notifyItemChange(QString localPath, int newState)
{
    QByteArray response;
    string command = "P:";
    command += localPath.toStdString();

    if (newState == MegaApi::STATE_PENDING
            || newState == MegaApi::STATE_SYNCED
            || newState == MegaApi::STATE_SYNCING)
    {
        command.append(":");
        command.append(QString::number(newState).toUtf8().constData());
        command.append(":");
        if (Preferences::instance()->overlayIconsDisabled()) // Respond to extension to not show badges
        {
            command.append("0");
        }
        else // Respond to extension to show badges
        {
            command.append("1");
        }
        emit sendToAll(QByteArray(command.data(), command.size()));
    }
}

void MacXExtServer::notifySyncAdd(QString path, QString syncName)
{
    emit sendToAll((QString::fromUtf8("A:")
                   + path
                   + QChar::fromAscii(':')
                   + syncName).toUtf8());
}

void MacXExtServer::notifySyncDel(QString path, QString syncName)
{
    emit sendToAll((QString::fromUtf8("D:")
                   + path
                   + QChar::fromAscii(':')
                   + syncName).toUtf8());
}

void MacXExtServer::notifyAllClients(int op)
{
    // send the list of current synced folders to all connected clients
    // This is needed once MEGAsync switches from non-logged to logged state and vice-versa

    SyncInfo *model = SyncInfo::instance();
    QString command;
    if (op == NOTIFY_ADD_SYNCS)
    {
        command = QString::fromUtf8("A:");
    }
    else if (op == NOTIFY_DEL_SYNCS)
    {
        command = QString::fromUtf8("D:");
    }

    for (auto syncSetting : model->getAllSyncSettings())
    {
        QString syncPath = QDir::toNativeSeparators(QDir(syncSetting->getLocalFolder()).canonicalPath());
        if (!syncPath.size() || syncSetting->getRunState() != MegaSync::RUNSTATE_RUNNING)
        {
            continue;
        }

        QString message = command + syncPath + QChar::fromAscii(':') + syncSetting->name(true);

        emit sendToAll(message.toUtf8());
    }
}
