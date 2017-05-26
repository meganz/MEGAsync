#include "MacXExtServer.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

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

    connect(this, SIGNAL(sendToAll(QString )), this, SLOT(doSendToAll(QString)));
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), app, SLOT(shellUpload(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), app, SLOT(shellExport(QQueue<QString>)),Qt::QueuedConnection);
    connect(this, SIGNAL(viewOnMega(QString)), app, SLOT(shellViewOnMega(QString)),Qt::QueuedConnection);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

MacXExtServer::~MacXExtServer()
{
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

        connect(client, SIGNAL(dataReady()), this, SLOT(onClientData()));
        connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));
        m_clients.append(client);

        // send the list of current synced folders to the new client
        Preferences *preferences = Preferences::instance();
        for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            QString syncPath = QDir::toNativeSeparators(QDir(preferences->getLocalFolder(i)).canonicalPath());
            if (!syncPath.size() || !preferences->isFolderActive(i))
            {
                continue;
            }

            QString message = QString::fromUtf8("A:") + syncPath + QDir::separator()
                    + QChar::fromAscii(':') + preferences->getSyncName(i);
            client->writeData(message.toUtf8().constData(), message.length());
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
            client->writeData(buf.constData(), buf.size());
        }
        buf.clear();
        response.clear();
    }
}

void MacXExtServer::onClientDisconnected()
{
    MacXLocalSocket *client = qobject_cast<MacXLocalSocket *>(sender());
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
            if (!ok || numFiles <= 0)
            {
                break;
            }

            int numFolders = parameters[2].toInt(&ok);
            if (!ok || numFolders <= 0)
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
            if (Preferences::instance()->overlayIconsDisabled())
            {
                return false;
            }

            std::string tmpPath(content);

            if (!tmpPath.empty() && tmpPath[tmpPath.length() - 1] == '/')
            {
                tmpPath.erase(tmpPath.length() - 1, 1);
            }

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
            QString filePath = QString::fromUtf8(content);
            QFileInfo file(filePath);
            if (file.exists())
            {
                emit viewOnMega(filePath);
            }
            return false;
        }
        default:
            break;
    }
    return false;
}

void MacXExtServer::doSendToAll(QString str)
{
    for (int i = 0; i < m_clients.size(); i++)
    {
        MacXLocalSocket *socket = m_clients[i];
        if (socket)
        {
            socket->writeData(str.toUtf8().constData(), str.length());
        }
    }
}

void MacXExtServer::notifyItemChange(QString path)
{
    QByteArray response;
    QString command = QString::fromUtf8("P:") + path;

    if(QDir(path).exists())
    {
        command += QDir::separator();
    }

    bool shouldRespond = GetAnswerToRequest(command.toStdString().c_str(), &response);
    if (shouldRespond)
    {
        command.append(QChar::fromAscii(':'));
        command.append(QString::fromUtf8(response.data()));
        emit sendToAll(command);
    }
}

void MacXExtServer::notifySyncAdd(QString path, QString syncName)
{
    if(QDir(path).exists())
    {
        path += QDir::separator();
    }

    emit sendToAll(QString::fromUtf8("A:")
                   + path
                   + QChar::fromAscii(':')
                   + syncName);
}

void MacXExtServer::notifySyncDel(QString path, QString syncName)
{
    emit sendToAll(QString::fromUtf8("D:")
                   + path
                   + QChar::fromAscii(':')
                   + syncName);
}
