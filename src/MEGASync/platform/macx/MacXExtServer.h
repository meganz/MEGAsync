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
   template <class COMMAND>
   void addIsIncomingShareToCommand(std::string* path, COMMAND* command)
   {
       command->append(":");
       std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getSyncedNode(path));
       if(Utilities::isIncommingShare(node.get()))
       {
           command->append("1");
       }
       else
       {
           command->append("0");
       }
   }

   template <class COMMAND>
   void addOverlayIconsDisabledToCommand(COMMAND* command)
   {
       command->append(":");
       if (Preferences::instance()->overlayIconsDisabled()) // Respond to extension to not show badges
       {
           command->append("0");
       }
       else // Respond to extension to show badges
       {
           command->append("1");
       }
   }


signals:
   void newUploadQueue(QQueue<QString> uploadQueue);
   void newExportQueue(QQueue<QString> exportQueue);
   void viewOnMega(QByteArray localPath, bool versions);
};

Q_DECLARE_METATYPE(QPointer<MacXLocalSocket>);

#endif // MACXEXTSERVER_H
