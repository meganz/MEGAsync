#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include <QLocalSocket>
#include <QLocalServer>
#include <QXmlStreamWriter>

#include "megaapi.h"

#define LOGS_FOLDER_LEAFNAME_QSTRING QString::fromUtf8("logs")

struct LoggingThread;
class MegaSyncLogger : public QObject, public mega::MegaLogger
{
    Q_OBJECT

public:
    MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& mDesktopPath, bool logToStdout);
    ~MegaSyncLogger();
    void log(const char *time, int loglevel, const char *source, const char *message
#ifdef ENABLE_LOG_PERFORMANCE
             , const char **directMessages, size_t *directMessagesSizes, int numberMessages
#endif
             ) override;
    void setDebug(bool enable);
    bool isDebug() const;
    bool mLogToStdout = false;

    // this one is called on signal (flush log before crash report)
    void flushAndClose();

    /**
     * @brief prepareForReporting
     * Prepare for reporting. Will pause logs and force a rotation.
     * Once the logs are rotated, a logReadyForReporting signal will be emitted.
     * Once logs are reported, call resumeAfterReporting.
     * @returns true if preparation went well (if false, there is no need for resumeAfterReporting)
     */
    bool prepareForReporting();
    bool cleanLogs();
    void resumeAfterReporting();

signals:
    void logReadyForReporting();
    void logCleaned();

private:
    static void exitFunction();

    QString mDesktopPath;
    std::unique_ptr<LoggingThread> g_loggingThread;
};

extern MegaSyncLogger *g_megaSyncLogger;   // for crash report flush
