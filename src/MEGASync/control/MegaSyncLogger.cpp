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
const char* MEGA_LOG_FILENAME = "/MEGAsync.log";

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

    constexpr auto maxFileSizeMB = 10;
    constexpr auto maxFileCount = 10;
    auto rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                dataPath.toStdString() + MEGA_LOG_FILENAME, 1024 * 1024 * maxFileSizeMB, maxFileCount);
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

    spdlog::register_logger(mLogger);
    spdlog::flush_every(std::chrono::seconds{3});
    spdlog::flush_on(spdlog::level::err);
}

MegaSyncLogger::~MegaSyncLogger()
{
    mRunning = false;
    mLogger->flush();
    if (mDebugLogger)
    {
        mDebugLogger->flush();
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
    if (!mRunning)
    {
        return;
    }

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

    if (mDebug)
    {
        switch (loglevel)
        {
            case MegaApi::LOG_LEVEL_FATAL: mDebugLogger->critical(message); break;
            case MegaApi::LOG_LEVEL_ERROR: mDebugLogger->error(message); break;
            case MegaApi::LOG_LEVEL_WARNING: mDebugLogger->warn(message); break;
            case MegaApi::LOG_LEVEL_INFO: mDebugLogger->info(message); break;
            case MegaApi::LOG_LEVEL_DEBUG: mDebugLogger->debug(message); break;
            case MegaApi::LOG_LEVEL_MAX: mDebugLogger->trace(message); break;
        }
    }
}

void MegaSyncLogger::setDebug(const bool enable)
{
    mDebug = enable;
    if (enable)
    {
        if (!mDebugLogger)
        {
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(mDesktopPath.toStdString() + MEGA_LOG_FILENAME);
            std::vector<spdlog::sink_ptr> debugSinks{fileSink};
            mDebugLogger = std::make_shared<spdlog::async_logger>("debug_logger",
                                                                  debugSinks.begin(), debugSinks.end(),
                                                                  mThreadPool,
                                                                  spdlog::async_overflow_policy::overrun_oldest);
            mDebugLogger->set_pattern(MEGA_LOG_PATTERN, spdlog::pattern_time_type::utc);
            mDebugLogger->set_level(spdlog::level::trace);

            spdlog::register_logger(mDebugLogger);
        }
    }
    else
    {
        if (mDebugLogger)
        {
            mDebugLogger->flush();
        }
    }
}

bool MegaSyncLogger::isDebug() const
{
    return mDebug.load();
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
