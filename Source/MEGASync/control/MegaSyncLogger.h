#ifndef MEGASYNCLOGGER_H
#define MEGASYNCLOGGER_H

#include <QLocalSocket>
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

protected:
    QLocalSocket client;
    QXmlStreamWriter *xmlWriter;

};

#endif // MEGASYNCLOGGER_H
