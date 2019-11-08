#include "MegaSyncLogger.h"
#include "Utilities.h"

#include <fstream>
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

#include <zlib.h>

#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

namespace {

static bool awaiting_rotation = false;
MegaSyncLogger *megaSyncLogger = nullptr;

const char* MEGA_LOG_PATTERN = "%m-%dT%H:%M:%S.%f %t %l %v";

void onAllRotated()
{
    if (awaiting_rotation)
    {
        if (megaSyncLogger)
        {
            emit megaSyncLogger->logReadyForReporting();
        }
        awaiting_rotation = false;
    }
}

bool isGzipCompressed(const spdlog::filename_t& filename)
{
    std::ifstream file{filename, std::ios::binary}; // Windows: Rely on extension accepting a std::wstring
    if (!file.is_open())
    {
        return false;
    }
    std::array<char, 2> buf{};
    file.read(buf.data(), buf.size());
    if (!file.good())
    {
        return false;
    }
    return static_cast<unsigned char>(buf[0]) == 0x1f && static_cast<unsigned char>(buf[1]) == 0x8b; // checks for gzip bytes
}

void gzipCompressOnRotate(const spdlog::filename_t& filename)
{
    using spdlog::details::os::filename_to_str;

    if (isGzipCompressed(filename))
    {
        // ignore file if it's already compressed
        return;
    }

    std::ifstream file{filename}; // Windows: Rely on extension accepting a std::wstring
    if (!file.is_open())
    {
        std::cerr << "Unable to open log file for reading: " << filename_to_str(filename) << std::endl;
        return;
    }

#ifdef _WIN32
    const auto gzfilename = filename + L".gz";
    const auto gzopenFunc = gzopen_w;
#else
    const auto gzfilename = filename + ".gz";
    const auto gzopenFunc = gzopen;
#endif

    auto gzdeleter = [](gzFile_s* f) { if (f) gzclose(f); };

    std::unique_ptr<gzFile_s, decltype(gzdeleter)> gzfile{gzopenFunc(gzfilename.c_str(), "wb"), gzdeleter};
    if (!gzfile)
    {
        std::cerr << "Unable to open gzfile for writing: " << filename_to_str(gzfilename) << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        line.push_back('\n');
        if (gzputs(gzfile.get(), line.c_str()) == -1)
        {
            std::cerr << "Unable to compress log file: " << filename_to_str(filename) << std::endl;
            return;
        }
    }

    gzfile.reset();
    file.close();

    // rename e.g. MEGAsync.1.log.gz to MEGAsync.1.log (necessary for the rotation logic to work)
    spdlog::details::os::remove(filename);
    if (spdlog::details::os::rename(gzfilename, filename))
    {
        // if failed try again after a small delay.
        // this is a workaround to a windows issue, where very high rotation
        // rates can cause the rename to fail with permission denied (because of antivirus?).
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        spdlog::details::os::remove(filename);
        if (spdlog::details::os::rename(gzfilename, filename))
        {
            std::cerr << "Unable to rename from: " << filename_to_str(gzfilename) << " to: " << filename_to_str(filename) << std::endl;
            return;
        }
    }
}

}

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

    std::vector<spdlog::sink_ptr> sinks;

    constexpr auto maxFileSizeMB = 10;
    constexpr auto maxFileCount = 100;
    try
    {
#ifdef _WIN32
        rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    logPath.toStdWString(), 1024 * 1024 * maxFileSizeMB, maxFileCount, false, gzipCompressOnRotate, onAllRotated);
#else
        rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    logPath.toStdString(), 1024 * 1024 * maxFileSizeMB, maxFileCount, false, gzipCompressOnRotate, onAllRotated);
#endif
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception constructing rotating file sink: " << e.what() << std::endl;
    }

    if (rotatingFileSink)
    {
        sinks.push_back(rotatingFileSink);
    }

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

    mega::MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_MAX);
    mega::MegaApi::addLoggerObject(this);
}

MegaSyncLogger::~MegaSyncLogger()
{
    mega::MegaApi::removeLoggerObject(this);

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
    megaSyncLogger = nullptr;
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
        case mega::MegaApi::LOG_LEVEL_FATAL: mLogger->critical(message); break;
        case mega::MegaApi::LOG_LEVEL_ERROR: mLogger->error(message); break;
        case mega::MegaApi::LOG_LEVEL_WARNING: mLogger->warn(message); break;
        case mega::MegaApi::LOG_LEVEL_INFO: mLogger->info(message); break;
        case mega::MegaApi::LOG_LEVEL_DEBUG: mLogger->debug(message); break;
        case mega::MegaApi::LOG_LEVEL_MAX: mLogger->trace(message); break;
    }

    if (auto logger = std::atomic_load(&mDebugLogger))
    {
        switch (loglevel)
        {
            case mega::MegaApi::LOG_LEVEL_FATAL: logger->critical(message); break;
            case mega::MegaApi::LOG_LEVEL_ERROR: logger->error(message); break;
            case mega::MegaApi::LOG_LEVEL_WARNING: logger->warn(message); break;
            case mega::MegaApi::LOG_LEVEL_INFO: logger->info(message); break;
            case mega::MegaApi::LOG_LEVEL_DEBUG: logger->debug(message); break;
            case mega::MegaApi::LOG_LEVEL_MAX: logger->trace(message); break;
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
#ifdef _WIN32
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.toStdWString());
#else
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.toStdString());
#endif
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

bool MegaSyncLogger::prepareForReporting()
{
    if (!rotatingFileSink)
    {
        return false;
    }
    // The order here is important. We want spdlog to stop rotating,
    // and then force one last rotation, so as to have all the logs compressed so far
    // and we need to be notified when that last rotation finishes
    rotatingFileSink->pauseRotation = true; // this will prevent regular rotations from now on.
    megaSyncLogger = this;
    awaiting_rotation = true; //flag that we will use to emit a signal whenever there's a rotation from this moment on. i.e. the forced last rotation

    rotatingFileSink->forcerotation = true; //to make sure spdlog rotates on the next logged message
    mLogger->error("Preparing logger to send bug repport"); //To ensure rotation is performed!
    mLogger->flush(); //to ensure the above log is flushed and hence, the rotation takes place

    return true;
}

void MegaSyncLogger::resumeAfterReporting()
{
    if (!rotatingFileSink)
    {
        return;
    }
    rotatingFileSink->pauseRotation = false;
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
        case mega::MegaApi::LOG_LEVEL_DEBUG:
            level = QString::fromUtf8("debug");
            break;
        case mega::MegaApi::LOG_LEVEL_ERROR:
            level = QString::fromUtf8("error");
            break;
        case mega::MegaApi::LOG_LEVEL_FATAL:
            level = QString::fromUtf8("fatal");
            break;
        case mega::MegaApi::LOG_LEVEL_INFO:
            level = QString::fromUtf8("info");
            break;
        case mega::MegaApi::LOG_LEVEL_MAX:
            level = QString::fromUtf8("verbose");
            break;
        case mega::MegaApi::LOG_LEVEL_WARNING:
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
