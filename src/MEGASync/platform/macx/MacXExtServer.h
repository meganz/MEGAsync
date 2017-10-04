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

    enum {
        NOTIFY_ADD_SYNCS = 0,
        NOTIFY_DEL_SYNCS = 1
    };

    MacXExtServer(MegaApplication *app);
    virtual ~MacXExtServer();
    void notifyItemChange(std::string *localPath, int newState);
    void notifySyncAdd(QString path, QString syncName);
    void notifySyncDel(QString path, QString syncName);
    void notifyAllClients(int op);

protected:
    MacXLocalServer *m_localServer;
    QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;

public slots:
   void acceptConnection();
   void onClientData();
   void onClientDisconnected();

   void doSendToAll(QByteArray str);

private:
   QString sockPath;
   QList<MacXLocalSocket *> m_clients;
   bool GetAnswerToRequest(const char *buf, QByteArray *response);

signals:
   void newUploadQueue(QQueue<QString> uploadQueue);
   void newExportQueue(QQueue<QString> exportQueue);
   void viewOnMega(QByteArray localPath, bool versions);
   void sendToAll(QByteArray str);
};

#endif // MACXEXTSERVER_H
