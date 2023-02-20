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

protected:
    MacXLocalServer *m_localServer;
    QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;

public slots:
   void acceptConnection(QPointer<MacXLocalSocket> client);
   void onClientData(QByteArray data);

   void doSendToAll(QByteArray str);

   void notifyItemChange(QString localPath, int newState);
   void notifySyncAdd(QString path, QString syncName);
   void notifySyncDel(QString path, QString syncName);
   void notifyAllClients(int op);

private:
   QString sockPath;
   QList<QPointer<MacXLocalSocket>> m_clients;
   bool GetAnswerToRequest(const char *buf, QByteArray *response);
   void clientDisconnected(QPointer<MacXLocalSocket> client);

signals:
   void newUploadQueue(QQueue<QString> uploadQueue);
   void newExportQueue(QQueue<QString> exportQueue);
   void viewOnMega(QByteArray localPath, bool versions);
};

Q_DECLARE_METATYPE(QPointer<MacXLocalSocket>);

#endif // MACXEXTSERVER_H
