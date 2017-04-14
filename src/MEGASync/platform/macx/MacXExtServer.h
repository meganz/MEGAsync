#ifndef MACXEXTSERVER_H
#define MACXEXTSERVER_H

#include "MegaApplication.h"
#include "MacXLocalServer.h"
#include "MacXLocalSocket.h"

typedef enum {
   STRING_UPLOAD = 0,
   STRING_GETLINK = 1,
   STRING_SHARE = 2,
   STRING_SEND = 3
} StringID;

class MacXExtServer : public QObject
{
    Q_OBJECT

public:
    MacXExtServer(MegaApplication *app);
    virtual ~MacXExtServer();
    void notifyItemChange(QString path);
    void notifySyncAdd(QString path, QString syncName);
    void notifySyncDel(QString path, QString syncName);

protected:
    MacXLocalServer *m_localServer;
    QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;

public slots:
   void acceptConnection();
   void onClientData();
   void onClientDisconnected();

   void doSendToAll(QString str);

private:
   QString sockPath;
   QList<MacXLocalSocket *> m_clients;
   bool GetAnswerToRequest(const char *buf, QByteArray *response);

signals:
   void newUploadQueue(QQueue<QString> uploadQueue);
   void newExportQueue(QQueue<QString> exportQueue);
   void sendToAll(QString str);
};

#endif // MACXEXTSERVER_H
