#ifndef EXTSERVER_H
#define EXTSERVER_H

#include "MegaApplication.h"
#include "megaapi.h"
#include "control/Preferences.h"

typedef enum {
   STRING_UPLOAD = 0,
   STRING_GETLINK = 1,
   STRING_SHARE = 2,
   STRING_SEND = 3
} StringID;

class ExtServer: public QObject
{
    Q_OBJECT

 public:
    ExtServer(MegaApplication *app);
    virtual ~ExtServer();

 protected:
    QLocalServer *m_localServer;
	QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;

 public Q_SLOTS:
    void acceptConnection();
    void onClientData();
    void onClientDisconnected();
 private:
    QString sockPath;
    QList<QLocalSocket *> m_clients;
    const char *GetAnswerToRequest(const char *buf);

 signals:
	void newUploadQueue(QQueue<QString> uploadQueue);
    void newExportQueue(QQueue<QString> exportQueue);
};

#endif
