#ifndef MEGASYNCLOGGER_H
#define MEGASYNCLOGGER_H

#include <QLocalSocket>
#include <QLocalServer>
#include <QXmlStreamWriter>

#include "megaapi.h"

class MegaSyncLogger : public QObject, public mega::MegaLogger
{
    Q_OBJECT

public:
    MegaSyncLogger(QObject *parent = NULL);
    ~MegaSyncLogger();
    virtual void log(const char *time, int loglevel, const char *source, const char *message);
    void sendLogsToStdout(bool enable);
    void sendLogsToFile(bool enable);
    bool isLogToStdoutEnabled();
    bool isLogToFileEnabled();

signals:
    void sendLog(QString time, int loglevel, QString message);

public slots:
    void onLogAvailable(QString time, int loglevel, QString message);
    void clientConnected();
    void disconnected();

protected:
    QLocalSocket* client;
    QLocalServer* megaServer;
    QXmlStreamWriter *xmlWriter;
    bool connected;
    bool logToStdout;
    bool logToFile;
};

#endif // MEGASYNCLOGGER_H
