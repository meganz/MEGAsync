#include "MegaSyncLogger.h"
#include "Utilities.h"

#include <iostream>
#include <sstream>

#include <QFileInfo>
#include <QString>
#include <QDesktopServices>
#include <QDir>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/async.h>

#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

using namespace mega;
using namespace std;

MegaSyncLogger::MegaSyncLogger(QObject *parent) : QObject(parent), MegaLogger()
{
    xmlWriter = NULL;
    connected = true;
    logToStdout = false;
    logToFile = false;
    client = NULL;
    megaServer = NULL;

#ifdef LOG_TO_LOGGER
    QLocalServer::removeServer(ENABLE_MEGASYNC_LOGS);
    client = new QLocalSocket();
    megaServer = new QLocalServer(this);

    connect(megaServer,SIGNAL(newConnection()),this,SLOT(clientConnected()));
    connect(this, SIGNAL(sendLog(QString,int,QString)),
            this, SLOT(onLogAvailable(QString,int,QString)), Qt::QueuedConnection);
    connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));

    megaServer->listen(ENABLE_MEGASYNC_LOGS);
    client->connectToServer(MEGA_LOGGER);
#endif
}

MegaSyncLogger::~MegaSyncLogger()
{
    running = false;
    spdlog::shutdown();

    disconnected();

    if (megaServer)
    {
        delete megaServer;
    }
}

void MegaSyncLogger::init(const QString& dataPath)
{
    spdlog::init_thread_pool(8192, 1);
    spdlog::flush_every(std::chrono::seconds{3});
    spdlog::flush_on(spdlog::level::err);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(dataPath.toStdString() + "/MEGAsync.log", 1024*1024*10, 10);
    std::vector<spdlog::sink_ptr> sinks{rotating_sink};
    if (logToStdout)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
    logger = std::make_shared<spdlog::async_logger>("async_file_logger",
                                                    sinks.begin(), sinks.end(),
                                                    spdlog::thread_pool(),
                                                    spdlog::async_overflow_policy::overrun_oldest);
    logger->set_pattern("[%Y-%m-%dT%H:%M:%S.%u] [%l] [%t] %v");

    spdlog::register_logger(logger);
}

void MegaSyncLogger::log(const char*, int loglevel, const char*, const char *message)
{
    if (!running)
    {
        return;
    }

#ifdef LOG_TO_LOGGER
    if (connected) // TODO: Should be atomic
    {
        QString m = QString::fromUtf8(message);
        if (m.size() > MAX_MESSAGE_SIZE)
        {
            m = m.left(MAX_MESSAGE_SIZE - 3).append(QString::fromUtf8("..."));
        }

        auto t = std::time(NULL);
        char ts[50];
        if (!std::strftime(ts, sizeof(ts), "%H:%M:%S", std::gmtime(&t)))
        {
            ts[0] = '\0';
        }
        emit sendLog(QString::fromUtf8(ts), loglevel, m);
    }
#endif

    switch (loglevel)
    {
        case MegaApi::LOG_LEVEL_FATAL: logger->critical(message); break;
        case MegaApi::LOG_LEVEL_ERROR: logger->error(message); break;
        case MegaApi::LOG_LEVEL_WARNING: logger->warn(message); break;
        case MegaApi::LOG_LEVEL_INFO: logger->info(message); break;
        case MegaApi::LOG_LEVEL_DEBUG: logger->debug(message); break;
        case MegaApi::LOG_LEVEL_MAX: logger->trace(message); break;
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
