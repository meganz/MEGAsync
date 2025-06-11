#ifndef NOTIFYSERVER_H
#define NOTIFYSERVER_H

#include "MegaApplication.h"

class NotifyServer: public QObject
{
    Q_OBJECT

public:
    NotifyServer();
    virtual ~NotifyServer();
    void notifyItemChange(const std::string& localPath);
    void notifySyncAdd(QString path);
    void notifySyncDel(QString path);

 protected:
     QLocalServer* mLocalServer;

 public Q_SLOTS:
     void acceptConnection();
     void onClientDisconnected();
     void doSendToAll(const QByteArray& payload);

 private:
     enum class NotifyType : char
     {
         SyncAdded = 'A',
         SyncDeleted = 'D',
         ItemChanged = 'P',
         EndLine = '\n'
     };

     MegaApplication* app;
     QString mSockPath;
     QList<QLocalSocket*> mClients;
     QByteArray createPayload(NotifyType type, const QByteArray& data);

 signals:
     void sendToAll(const QByteArray& str);
};

#endif
