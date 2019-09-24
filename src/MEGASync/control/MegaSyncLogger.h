#pragma once

#include <QLocalSocket>
#include <QLocalServer>
#include <QXmlStreamWriter>

#include "megaapi.h"

#include <spdlog/spdlog.h>

class MegaSyncLogger : public QObject, public mega::MegaLogger
{
    Q_OBJECT

public:
    MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& mDesktopPath, bool logToStdout);
    ~MegaSyncLogger();
    void log(const char *time, int loglevel, const char *source, const char *message) override;
    void setDebug(bool enable);
    bool isDebug() const;

signals:
    void sendLog(QString time, int loglevel, QString message);

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
    std::atomic<bool> mDebug{false};
    std::atomic<bool> mRunning{true};
    std::shared_ptr<spdlog::details::thread_pool> mThreadPool;
    std::shared_ptr<spdlog::logger> mLogger;
    std::shared_ptr<spdlog::logger> mDebugLogger;
};
