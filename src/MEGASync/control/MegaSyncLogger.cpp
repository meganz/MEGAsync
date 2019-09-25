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
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/async.h>

#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

namespace {

const char* MEGA_LOG_PATTERN = "%m-%dT%H:%M:%S.%e %L %t %v";

}

using namespace mega;
using namespace std;

MegaSyncLogger::MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& desktopPath, bool logToStdout)
: QObject{parent}
, mDesktopPath{desktopPath}
, mThreadPool{std::make_shared<spdlog::details::thread_pool>(8192, 1, []{})} // Queue size of 8192 and 1 thread in pool
{
#ifdef LOG_TO_LOGGER
    QLocalServer::removeServer(ENABLE_MEGASYNC_LOGS);
    mClient = new QLocalSocket();
    mMegaServer = new QLocalServer(this);

    connect(mMegaServer,SIGNAL(newConnection()),this,SLOT(clientConnected()));
    connect(this, SIGNAL(sendLog(QString,int,QString)),
            this, SLOT(onLogAvailable(QString,int,QString)), Qt::QueuedConnection);
    connect(mClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mClient, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));

    mMegaServer->listen(ENABLE_MEGASYNC_LOGS);
    mClient->connectToServer(MEGA_LOGGER);
#endif

    const QDir dataDir{dataPath};
    dataDir.mkdir(QString::fromUtf8("logs"));
    const auto logPath = dataDir.filePath(QString::fromUtf8("logs/MEGAsync.log"));

    constexpr auto maxFileSizeMB = 10;
    constexpr auto maxFileCount = 10;
    auto rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath.toStdString(), 1024 * 1024 * maxFileSizeMB, maxFileCount);
    std::vector<spdlog::sink_ptr> sinks{rotatingFileSink};
    if (logToStdout)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
    mLogger = std::make_shared<spdlog::async_logger>("logger",
                                                     sinks.begin(), sinks.end(),
                                                     mThreadPool,
                                                     spdlog::async_overflow_policy::overrun_oldest);
    mLogger->set_pattern(MEGA_LOG_PATTERN, spdlog::pattern_time_type::utc);
    mLogger->set_level(spdlog::level::trace);
    mLogger->flush_on(spdlog::level::err);

    spdlog::register_logger(mLogger);
}

MegaSyncLogger::~MegaSyncLogger()
{
    mLogger->flush();
    if (auto logger = std::atomic_load(&mDebugLogger))
    {
        logger->flush();
    }
    spdlog::shutdown();

    disconnected();

    if (mMegaServer)
    {
        delete mMegaServer;
    }
}

void MegaSyncLogger::log(const char*, int loglevel, const char*, const char *message)
{
#ifdef LOG_TO_LOGGER
    if (mConnected)
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
        case MegaApi::LOG_LEVEL_FATAL: mLogger->critical(message); break;
        case MegaApi::LOG_LEVEL_ERROR: mLogger->error(message); break;
        case MegaApi::LOG_LEVEL_WARNING: mLogger->warn(message); break;
        case MegaApi::LOG_LEVEL_INFO: mLogger->info(message); break;
        case MegaApi::LOG_LEVEL_DEBUG: mLogger->debug(message); break;
        case MegaApi::LOG_LEVEL_MAX: mLogger->trace(message); break;
    }

    if (auto logger = std::atomic_load(&mDebugLogger))
    {
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
}

void MegaSyncLogger::setDebug(const bool enable)
{
    if (enable)
    {
        if (!std::atomic_load(&mDebugLogger))
        {
            const QDir desktopDir{mDesktopPath};
            const auto logPath = desktopDir.filePath(QString::fromUtf8("MEGAsync.log"));

            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.toStdString());
            std::vector<spdlog::sink_ptr> debugSinks{fileSink};
            std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::async_logger>(
                                                        "debug_logger",
                                                         debugSinks.begin(), debugSinks.end(),
                                                         mThreadPool,
                                                         spdlog::async_overflow_policy::overrun_oldest);
            logger->set_pattern(MEGA_LOG_PATTERN, spdlog::pattern_time_type::utc);
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::err);

            spdlog::register_logger(logger);
            std::atomic_store(&mDebugLogger, logger);
        }
    }
    else
    {
        if (auto logger = std::atomic_load(&mDebugLogger))
        {
            logger->flush();

            spdlog::drop(logger->name());
            std::atomic_store(&mDebugLogger, std::shared_ptr<spdlog::logger>{});
        }
    }
}

bool MegaSyncLogger::isDebug() const
{
    return std::atomic_load(&mDebugLogger) != nullptr;
}

void MegaSyncLogger::onLogAvailable(QString time, int loglevel, QString message)
{
    if (!mConnected)
    {
        return;
    }

    if (!mXmlWriter)
    {
        mXmlWriter = new QXmlStreamWriter(mClient);
        mXmlWriter->writeStartDocument();
        mXmlWriter->writeStartElement(QString::fromUtf8("MEGA"));

        mXmlWriter->writeStartElement(QString::fromUtf8("log"));
        mXmlWriter->writeAttribute(QString::fromUtf8("timestamp"), time);
        mXmlWriter->writeAttribute(QString::fromUtf8("type"), QString::fromUtf8("info"));
        mXmlWriter->writeAttribute(QString::fromUtf8("content"), QString::fromUtf8("LOG START"));
        mXmlWriter->writeEndElement();
    }

    if (mXmlWriter->hasError())
    {
        mConnected = false;
        delete mXmlWriter;
        mXmlWriter = nullptr;
        mClient->deleteLater();
        mClient = nullptr;
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

    mXmlWriter->writeStartElement(QString::fromUtf8("log"));
    mXmlWriter->writeAttribute(QString::fromUtf8("timestamp"), time);
    mXmlWriter->writeAttribute(QString::fromUtf8("type"),level);
    mXmlWriter->writeAttribute(QString::fromUtf8("content"), message);
    mXmlWriter->writeEndElement();
    mClient->flush();
}

void MegaSyncLogger::clientConnected()
{
    disconnected();

    while (mMegaServer->hasPendingConnections())
    {
        QLocalSocket *socket = mMegaServer->nextPendingConnection();
        socket->disconnectFromServer();
        socket->deleteLater();
    }

    mClient = new QLocalSocket();
    connect(mClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mClient, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));
    mClient->connectToServer(MEGA_LOGGER);
    mConnected = true;
}

void MegaSyncLogger::disconnected()
{
    mConnected = false;
    if (mXmlWriter)
    {
        delete mXmlWriter;
        mXmlWriter = NULL;
    }

    if (mClient)
    {
        mClient->deleteLater();
        mClient = NULL;
    }
}
