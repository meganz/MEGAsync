#pragma once

#include <atomic>
#include <memory>

#include <QLocalSocket>
#include <QLocalServer>
#include <QXmlStreamWriter>

#include "megaapi.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace spdlog {
class logger;
namespace details {
class thread_pool;
}
}

class MegaSyncLogger : public QObject, public mega::MegaLogger
{
    Q_OBJECT

public:
    MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& mDesktopPath, bool logToStdout);
    ~MegaSyncLogger();
    void log(const char *time, int loglevel, const char *source, const char *message) override;
    void setDebug(bool enable);
    bool isDebug() const;

    /**
     * @brief prepareForReporting
     * Prepare for reporting. Will pause logs and force a rotation.
     * Once the logs are rotated, a logReadyForReporting signal will be emitted.
     * Once logs are reported, call resumeAfterReporting.
     * @returns true if preparation went well (if false, there is no need for resumeAfterReporting)
     */
    bool prepareForReporting();
    void resumeAfterReporting();

signals:
    void sendLog(QString time, int loglevel, QString message);
    void logReadyForReporting();

public slots:
    void onLogAvailable(QString time, int loglevel, QString message);
    void clientConnected();
    void disconnected();

private:
    QString mDesktopPath;
    QLocalSocket* mClient = nullptr;
    QLocalServer* mMegaServer = nullptr;
    QXmlStreamWriter* mXmlWriter = nullptr;
    std::atomic<bool> mConnected{true};
    std::shared_ptr<spdlog::details::thread_pool> mThreadPool;
    std::shared_ptr<spdlog::logger> mLogger; // Always-on logger with rotated file + stdout logging
    std::shared_ptr<spdlog::logger> mDebugLogger; // Logger used in debug mode (when toggling to debug)

    std::shared_ptr<spdlog::sinks::rotating_file_sink<std::mutex>> rotatingFileSink;
};
