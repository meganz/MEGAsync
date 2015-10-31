#include "MegaSyncLogger.h"
#include <iostream>
#include <sstream>

#include <QFileInfo>
#include <QString>
#include <QDesktopServices>
#include <QDir>

#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

using namespace mega;
using namespace std;

MegaSyncLogger::MegaSyncLogger() : QObject(), MegaLogger()
{
    xmlWriter = NULL;
    connected = true;
    logToStdout = false;
    logToFile = false;

#ifdef LOG_TO_LOGGER
    QLocalServer::removeServer(ENABLE_MEGASYNC_LOGS);
    client = new QLocalSocket();
    megaServer = new QLocalServer();

    connect(megaServer,SIGNAL(newConnection()),this,SLOT(clientConnected()));
    connect(this, SIGNAL(sendLog(QString,int,QString)),
            this, SLOT(onLogAvailable(QString,int,QString)), Qt::QueuedConnection);
    connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));

    megaServer->listen(ENABLE_MEGASYNC_LOGS);
    client->connectToServer(MEGA_LOGGER);
#endif
}

void MegaSyncLogger::log(const char *time, int loglevel, const char *source, const char *message)
{
#ifdef LOG_TO_LOGGER
    if (connected)
    {
        QString m = QString::fromUtf8(message);
        if (m.size() > MAX_MESSAGE_SIZE)
        {
            m = m.left(MAX_MESSAGE_SIZE - 3).append(QString::fromUtf8("..."));
        }

#ifdef DEBUG
        QString fileName;
        QFileInfo info(QString::fromUtf8(source));
        fileName = info.fileName();
        if (fileName.size())
        {
            m.append(QString::fromUtf8(" (%1)").arg(fileName));
        }
#endif

        emit sendLog(QString::fromUtf8(time), loglevel, m);
    }
#endif

    if (logToFile || logToStdout)
    {
        QString fileName;
        QFileInfo info(QString::fromUtf8(source));
        fileName = info.fileName();

        ostringstream oss;
        oss << time;
        switch(loglevel)
        {
            case MegaApi::LOG_LEVEL_DEBUG:
                oss << " (debug): ";
                break;
            case MegaApi::LOG_LEVEL_ERROR:
                oss << " (error): ";
                break;
            case MegaApi::LOG_LEVEL_FATAL:
                oss << " (fatal): ";
                break;
            case MegaApi::LOG_LEVEL_INFO:
                oss << " (info):  ";
                break;
            case MegaApi::LOG_LEVEL_MAX:
                oss << " (verb):  ";
                break;
            case MegaApi::LOG_LEVEL_WARNING:
                oss << " (warn):  ";
                break;
        }

        oss << message;
        if (fileName.size())
        {
            oss << " ("<< fileName.toStdString() << ")";
        }

        if (logToStdout)
        {
            cout << oss.str() << endl;
        }

        if (logToFile)
        {
            static QString filePath;
            if (filePath.isEmpty())
            {
                QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
                filePath = dataPath + QDir::separator() + QString::fromAscii("MEGAsync.log");
            }

            QFile file(filePath);
            file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            QTextStream out(&file);
            out << oss.str().c_str() << endl;
        }
    }
}

void MegaSyncLogger::sendLogsToStdout(bool enable)
{
    this->logToStdout = enable;
}

void MegaSyncLogger::sendLogsToFile(bool enable)
{
    this->logToFile = enable;
}

bool MegaSyncLogger::isLogToStdoutEnabled()
{
    return logToStdout;
}

bool MegaSyncLogger::isLogToFileEnabled()
{
    return logToFile;
}

void MegaSyncLogger::onLogAvailable(QString time, int loglevel, QString message)
{
    if (!connected)
    {
        return;
    }

    if (!xmlWriter)
    {
        xmlWriter = new QXmlStreamWriter(client);
        xmlWriter->writeStartDocument();
        xmlWriter->writeStartElement(QString::fromUtf8("MEGA"));

        xmlWriter->writeStartElement(QString::fromUtf8("log"));
        xmlWriter->writeAttribute(QString::fromUtf8("timestamp"), time);
        xmlWriter->writeAttribute(QString::fromUtf8("type"), QString::fromUtf8("info"));
        xmlWriter->writeAttribute(QString::fromUtf8("content"), QString::fromUtf8("LOG START"));
        xmlWriter->writeEndElement();
    }

    if (xmlWriter->hasError())
    {
        connected = false;
        delete xmlWriter;
        xmlWriter = NULL;
        client->deleteLater();
        client = NULL;
        return;
    }

    QString level;
    switch(loglevel)
    {
        case MegaApi::LOG_LEVEL_DEBUG:
            level = QString::fromUtf8("debug");
            break;
        case MegaApi::LOG_LEVEL_ERROR:
            level = QString::fromUtf8("error");
            break;
        case MegaApi::LOG_LEVEL_FATAL:
            level = QString::fromUtf8("fatal");
            break;
        case MegaApi::LOG_LEVEL_INFO:
            level = QString::fromUtf8("info");
            break;
        case MegaApi::LOG_LEVEL_MAX:
            level = QString::fromUtf8("verbose");
            break;
        case MegaApi::LOG_LEVEL_WARNING:
            level = QString::fromUtf8("warning");
            break;
        default:
            level = QString::fromUtf8("unknown");
            break;
    }

    xmlWriter->writeStartElement(QString::fromUtf8("log"));
    xmlWriter->writeAttribute(QString::fromUtf8("timestamp"), time);
    xmlWriter->writeAttribute(QString::fromUtf8("type"),level);
    xmlWriter->writeAttribute(QString::fromUtf8("content"), message);
    xmlWriter->writeEndElement();
    client->flush();
}

void MegaSyncLogger::clientConnected()
{
    disconnected();

    while (megaServer->hasPendingConnections())
    {
        QLocalSocket *socket = megaServer->nextPendingConnection();
        socket->disconnectFromServer();
        socket->deleteLater();
    }

    client = new QLocalSocket();
    connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));
    client->connectToServer(MEGA_LOGGER);
    connected = true;
}

void MegaSyncLogger::disconnected()
{
    connected = false;
    if (xmlWriter)
    {
        delete xmlWriter;
        xmlWriter = NULL;
    }

    if (client)
    {
        client->deleteLater();
        client = NULL;
    }
}
