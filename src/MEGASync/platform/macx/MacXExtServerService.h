#ifndef MACXEXTSERVERSERVICE_H
#define MACXEXTSERVERSERVICE_H

#include <QObject>
#include <QPointer>
#include "platform/macx/MacXExtServer.h"

/**
 * @brief Service instance to manage communications with extension
 *
 * Class that registers a MacXExtServer instance and offers some public signals that will be connected to worker object (MacXExtServer),
 * letting us use singal + slots across threads to improve performance.
 */
class MacXExtServerService : public QObject
{
    Q_OBJECT
public:
    MacXExtServerService(MegaApplication *receiver);
    ~MacXExtServerService();

signals:
    void syncAdd(QString path, QString syncName);
    void syncDel(QString path, QString syncName);
    void allClients(int op);
    void itemChange(QString localPath, int newState);

private:
    QThread mThreadExtServer;
    QPointer<MacXExtServer> mExtServer;
};

#endif // MACXEXTSERVERSERVICE_H
