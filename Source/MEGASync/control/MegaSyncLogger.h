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
    MegaSyncLogger();
    virtual void log(const char *time, int loglevel, const char *source, const char *message);

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
};

#endif // MEGASYNCLOGGER_H
